#ifndef DESCRIPTIONPANEL_HPP
#define DESCRIPTIONPANEL_HPP

#include <QWidget>
#include <QVariant>
#include <boost/optional.hpp>
#include <sstream>
#include <nix.hpp>
#include <QModelIndex>

namespace Ui {
class DescriptionPanel;
}

class DescriptionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DescriptionPanel(QWidget *parent = 0);
    ~DescriptionPanel();

public slots:
    void update_description_panel(QModelIndex qml);
    void clear_description_panel();

private:
    Ui::DescriptionPanel *ui;
};

#endif // DESCRIPTIONPANEL_HPP
