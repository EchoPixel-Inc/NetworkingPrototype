#include "clientApp/peerDelegate.h"

#include <QPainter>
#include <QStyle>
#include <QStylePainter>

namespace
{
constexpr double normalizedMarginWidth{1.0};  // margin width expresssed as a
// fraction of the current (average) character width

constexpr double normalizedCircleRadius{
	1.5};  // circle graphic radius expressed
// as a fraction of the current (average) character width

}  // namespace
//=============================================================================
PeerDelegate::PeerDelegate(QObject* parent) : QAbstractItemDelegate{parent} {}
//=============================================================================

//=============================================================================
void PeerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	QStyleOptionViewItem opt(option);

	const QPalette& palette(opt.palette);
	const QRect& rect(opt.rect);

	const bool lastIndex = (index.model()->rowCount() - 1) == index.row();
	const bool hasIcon = !opt.icon.isNull();
	const int bottomEdge = rect.bottom();
	QFont f(opt.font);

	painter->save();
	painter->setClipping(true);
	painter->setClipRect(rect);
	painter->setFont(opt.font);
	painter->setRenderHint(QPainter::Antialiasing);

	QLinearGradient gradient(
		opt.rect.left(), opt.rect.top(), opt.rect.left(), opt.rect.bottom());
	gradient.setColorAt(0, palette.button().color());
	gradient.setColorAt(1.0, palette.dark().color());

	QBrush brush(gradient);
	brush.setStyle(Qt::BrushStyle::LinearGradientPattern);
	painter->setBrush(brush);

	if (qvariant_cast<bool>(index.data(Qt::UserRole + 2))) {
		painter->fillRect(rect, QGradient::Preset::NightSky);
	}
	else {
		painter->fillRect(rect, brush);
	}

	painter->setPen(palette.dark().color());
	painter->drawLine(rect.left(), bottomEdge, rect.right(), bottomEdge);

	auto marginSize = static_cast<int>(
		normalizedMarginWidth * QFontMetrics(f).averageCharWidth());

	auto circleRadius = static_cast<int>(
		normalizedCircleRadius * QFontMetrics(f).averageCharWidth());

	QBrush b;
	b.setColor(qvariant_cast<QColor>(index.data(Qt::DecorationRole)));
	b.setStyle(Qt::SolidPattern);
	painter->setBrush(b);
	painter->setPen(Qt::NoPen);
	painter->drawEllipse({static_cast<double>(2 * marginSize + option.rect.x()),
							 option.rect.y() + 0.5 * option.rect.height()},
		static_cast<double>(circleRadius), static_cast<double>(circleRadius));

	auto nameRect =
		QFontMetrics(f).boundingRect(index.data(Qt::DisplayRole).toString());
	nameRect.moveTo(option.rect.x() + 2 * (marginSize + circleRadius),
		2 * marginSize + option.rect.y());

	painter->setFont(f);
	painter->setPen(palette.text().color());
	painter->drawText(
		nameRect, Qt::TextSingleLine, index.data(Qt::DisplayRole).toString());

	painter->restore();
}
//=============================================================================

//=============================================================================
QSize PeerDelegate::sizeHint(
	const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt(option);

	QFont f(opt.font);

	auto marginSize = static_cast<int>(
		normalizedMarginWidth * QFontMetrics(f).averageCharWidth());

	auto circleRadius = static_cast<int>(
		normalizedCircleRadius * QFontMetrics(f).averageCharWidth());

	auto nameRect =
		QFontMetrics(f).boundingRect(index.data(Qt::DisplayRole).toString());

	auto width =
		2 * (marginSize + circleRadius) + nameRect.width() + marginSize;
	auto height = 4 * marginSize + std::max(circleRadius, nameRect.height());

	return QSize(width, height);
}
//=============================================================================

//=============================================================================