// Reference: http://qt-project.org/faq/answer/how_can_i_align_the_checkboxes_in_a_view

#ifndef CENTEREDCHECKBOXDELEGATE_HPP
#define CENTEREDCHECKBOXDELEGATE_HPP

// Qt includes.
#include <QStyledItemDelegate>
class QPainter;

class CenteredCheckBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	private:
		typedef QStyledItemDelegate super;
		Q_DISABLE_COPY(CenteredCheckBoxDelegate)

	public:
		explicit CenteredCheckBoxDelegate(QObject *parent = 0);
 
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
				   const QModelIndex &index) const final;

		virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
				const QStyleOptionViewItem &option,
				const QModelIndex &index) final;
};
 
#endif /* CENTEREDCHECKBOXDELEGATE_HPP */
