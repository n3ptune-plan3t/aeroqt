#include <QStylePlugin>
#include "aerostyle.h"

class AeroQtPlugin : public QStylePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "aeroqt.json")
public:
    QStyle *create(const QString &key) override
    {
        return key.compare(QLatin1String("aeroqt"), Qt::CaseInsensitive) == 0 ? new AeroStyle : nullptr;
    }
};

#include "plugin.moc"
