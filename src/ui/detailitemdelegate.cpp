﻿#include "include/ui/detailitemdelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QRect>
#include <QBrush>

DetailItemDelegate::DetailItemDelegate(DetailWindowSetting* detailWindowSetting,QObject *parent) :
    WordItemDelegate(parent)
{
    this->detailWindowSetting = detailWindowSetting;
}

void DetailItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto options = option;
    initStyleOption(&options, index);

    painter->save();

    QRect rect(option.rect.left(), option.rect.top(),\
             option.rect.width(), option.rect.height());
    QBrush brush(Qt::gray);
    painter->fillRect(rect, brush);


    const DetailViewerModel * detailViewerModel = qobject_cast<const DetailViewerModel*>(index.model());
    //vector<pair<GameActions,float>> strategy = detailViewerModel->tableStrategyModel->get_strategy(this->detailWindowSetting->grid_i,this->detailWindowSetting->grid_j);

    options.text = "";
    if(detailViewerModel->tableStrategyModel->treeItem != NULL &&
            detailViewerModel->tableStrategyModel->treeItem->m_treedata.lock()->getType() == GameTreeNode::GameTreeNode::ACTION){
        shared_ptr<GameTreeNode> node = detailViewerModel->tableStrategyModel->treeItem->m_treedata.lock();
        int strategy_number = 0;
        if(this->detailWindowSetting->grid_i >= 0 && this->detailWindowSetting->grid_j >= 0){
           strategy_number = detailViewerModel->tableStrategyModel->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j].size();
        }
        int ind = index.row() * detailViewerModel->columns + index.column();

        if(ind < strategy_number){

            pair<int,int> strategy_ui_table = detailViewerModel->tableStrategyModel->ui_strategy_table[this->detailWindowSetting->grid_i][this->detailWindowSetting->grid_j][ind];
            int card1 = strategy_ui_table.first;
            int card2 = strategy_ui_table.second;
            vector<float> strategy = detailViewerModel->tableStrategyModel->current_strategy[card1][card2];


            shared_ptr<ActionNode> actionNode = dynamic_pointer_cast<ActionNode>(node);

            vector<GameActions>& gameActions = actionNode->getActions();
            if(gameActions.size() != strategy.size())throw runtime_error("size mismatch in DetailItemItemDelegate paint");
            float fold_prob = 0;
            vector<float> strategy_without_fold;
            float strategy_without_fold_sum = 0;
            for(int i = 0;i < strategy.size();i ++){
                GameActions one_action = gameActions[i];
                if(one_action.getAction() == GameTreeNode::PokerActions::FOLD){
                    fold_prob = strategy[i];
                }else{
                    strategy_without_fold.push_back(strategy[i]);
                    strategy_without_fold_sum += strategy[i];
                }
            }

            for(int i = 0;i < strategy_without_fold.size();i ++){
                strategy_without_fold[i] = strategy_without_fold[i] / strategy_without_fold_sum;
            }

            int disable_height = (int)(fold_prob * option.rect.height());
            int remain_height = option.rect.height() - disable_height;

            // draw background for flod
            QRect rect(option.rect.left(), option.rect.top(),\
                 option.rect.width(), disable_height);
            QBrush brush(QColor	(0,191,255));
            painter->fillRect(rect, brush);

            int ind = 0;
            float last_prob = 0;
            int bet_raise_num = 0;
            for(int i = 0;i < strategy.size();i ++){
                GameActions one_action = gameActions[i];
                QBrush brush(Qt::gray);
                if(one_action.getAction() != GameTreeNode::PokerActions::FOLD){
                if(one_action.getAction() == GameTreeNode::PokerActions::CHECK
                || one_action.getAction() == GameTreeNode::PokerActions::CALL){
                    brush = QBrush(Qt::green);
                }
                else if(one_action.getAction() == GameTreeNode::PokerActions::BET
                || one_action.getAction() == GameTreeNode::PokerActions::RAISE){
                    int color_base = max(128 - 32 * bet_raise_num - 1,0);
                    brush = QBrush(QColor(255,color_base,color_base));
                    bet_raise_num += 1;
                }else{
                brush = QBrush(Qt::blue);
                }

                int delta_x = (int)(option.rect.width() * last_prob);
                int delta_width = (int)(option.rect.width() * (last_prob + strategy_without_fold[ind])) - (int)(option.rect.width() * last_prob);

                QRect rect(option.rect.left() + delta_x, option.rect.top() + disable_height,\
                 delta_width , remain_height);
                painter->fillRect(rect, brush);

                last_prob += strategy_without_fold[ind];
                ind += 1;
                }
            }
            options.text = "";
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card1].toFormattedHtml();
            options.text += detailViewerModel->tableStrategyModel->cardint2card[card2].toFormattedHtml();
            options.text = "<h2>" + options.text + "<\/h2>";
            for(int i = 0;i < strategy.size();i ++){
                GameActions one_action = gameActions[i];
                float one_strategy = strategy[i] * 100;
                if(one_action.getAction() ==  GameTreeNode::PokerActions::FOLD){
                    options.text +=  QString(" <h5> %1 : %2\%<\/h5>").arg(tr("FOLD"),QString::number(one_strategy,'f',1));
                }
                else if(one_action.getAction() ==  GameTreeNode::PokerActions::CALL){
                    options.text +=  QString(" <h5> %1 : %2\%<\/h5>").arg(tr("CALL"),QString::number(one_strategy,'f',1));
                }
                else if(one_action.getAction() ==  GameTreeNode::PokerActions::CHECK){
                    options.text +=  QString(" <h5> %1 : %2\%<\/h5>").arg(tr("CHECK"),QString::number(one_strategy,'f',1));
                }
                else if(one_action.getAction() ==  GameTreeNode::PokerActions::BET){
                    options.text +=  QString(" <h5> %1 %2 : %3\%<\/h5>").arg(tr("BET"),QString::number(one_action.getAmount()),QString::number(one_strategy,'f',1));
                }
                else if(one_action.getAction() ==  GameTreeNode::PokerActions::RAISE){
                    options.text +=  QString(" <h5> %1 %2 : %3\%<\/h5>").arg(tr("RAISE"),QString::number(one_action.getAmount()),QString::number(one_strategy,'f',1));
                }
            }
        }
    }

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    //options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}
