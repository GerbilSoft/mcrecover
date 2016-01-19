// Reference: http://qt-project.org/faq/answer/how_can_i_align_the_checkboxes_in_a_view

#ifndef CENTEREDCHECKBOXDELEGATE_HPP
#define CENTEREDCHECKBOXDELEGATE_HPP

// Qt includes.
#include <QStyledItemDelegate>
class QPainter;

class CenteredCheckBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		CenteredCheckBoxDelegate(QObject *parent = 0);
 
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

		virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
				const QStyleOptionViewItem &option,
				const QModelIndex &index) override;
};
 
#endif /* CENTEREDCHECKBOXDELEGATE_HPP */
