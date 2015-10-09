#ifndef DESCRIPTIONPANEL_HPP
#define DESCRIPTIONPANEL_HPP

#include <QWidget>
#include <QVariant>
#include <boost/optional.hpp>
#include <sstream>
#include <nix.hpp>

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
    void update_description_panel(QVariant v);

private:
    Ui::DescriptionPanel *ui;

    template <typename T>
    void update(T arg);
};

#endif // DESCRIPTIONPANEL_HPP
