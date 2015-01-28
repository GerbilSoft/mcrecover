// Reference: http://qt-project.org/faq/answer/how_can_i_align_the_checkboxes_in_a_view

#include "CenteredCheckBoxDelegate.hpp"

// Qt includes.
#include <QtGui/QApplication>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>

CenteredCheckBoxDelegate::CenteredCheckBoxDelegate(QObject *parent)
        : QStyledItemDelegate(parent)
{ }

void CenteredCheckBoxDelegate::paint(QPainter *painter,
			const QStyleOptionViewItem &option,
			const QModelIndex &index) const
{
	// Check if this is a checkbox.
	if (!(index.flags() & Qt::ItemIsUserCheckable)) {
		// Not a checkbox.
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	// This is a checkbox.
	// TODO: Verify that "pressed" works correctly?

	// Draw the ItemView background.
	QStyleOptionViewItemV4 viewItemOption(option);
	QStyle *style = viewItemOption.widget ? viewItemOption.widget->style() : QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &viewItemOption, painter, viewItemOption.widget);

	// Calculate the checkbox parameters.
	// Reference: http://qt-project.org/forums/viewthread/14358
	QStyleOptionButton opt;
	opt.state = (index.data(Qt::CheckStateRole) == Qt::Checked)
			? QStyle::State_On
			: QStyle::State_Off;
	opt.state |= QStyle::State_Enabled;

	// Center the checkbox horizontally.
	opt.rect = QRect(0, 0, style->pixelMetric(QStyle::PM_IndicatorWidth), option.rect.height());
	opt.rect.moveCenter(option.rect.center());
	opt.rect.setHeight(style->pixelMetric(QStyle::PM_IndicatorHeight));
	style->drawControl(QStyle::CE_CheckBox, &opt, painter, viewItemOption.widget);
}
 
bool CenteredCheckBoxDelegate::editorEvent(QEvent *event,
		QAbstractItemModel *model,
		const QStyleOptionViewItem &option,
		const QModelIndex &index)
{
	Q_ASSERT(event);
	Q_ASSERT(model);

	// make sure that the item is checkable
	Qt::ItemFlags flags = model->flags(index);
	if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
		return false;
	// make sure that we have a check state
	QVariant value = index.data(Qt::CheckStateRole);
	if (!value.isValid())
		return false;
	// make sure that we have the right event type
	// TODO: Verify that "pressed" works correctly?
	if (event->type() == QEvent::MouseButtonRelease) {
		QStyleOptionViewItemV4 viewItemOption(option);
		QStyle *style = viewItemOption.widget ? viewItemOption.widget->style() : QApplication::style();

		// Calculate the checkbox rectangle.
		QRect checkRect(0, 0, style->pixelMetric(QStyle::PM_IndicatorWidth), option.rect.height());
		checkRect.moveCenter(option.rect.center());
		checkRect.setHeight(style->pixelMetric(QStyle::PM_IndicatorHeight));

		if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos()))
			return false;
	} else if (event->type() == QEvent::KeyPress) {
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
		    static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
		{
			return false;
		}
	} else {
		return false;
	}
	Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
				? Qt::Unchecked : Qt::Checked);
	return model->setData(index, state, Qt::CheckStateRole);
}
