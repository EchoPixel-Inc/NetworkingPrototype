#ifndef peerDelegate_h
#define peerDelegate_h

#include <QAbstractItemDelegate>

class PeerDelegate : public QAbstractItemDelegate
{
	Q_OBJECT;

public:
	explicit PeerDelegate(QObject* parent = nullptr);

	void paint(QPainter* painter, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override;

	QSize sizeHint(const QStyleOptionViewItem& option,
		const QModelIndex& index) const override;
};

#endif