#include "Window.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCommonStyle>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>

#include "Version.hpp"
#include "ab/Column.hpp"
#include "ab/Notebook.hpp"
#include "ab/Row.hpp"
#include "ab/util/MakeWidget.hpp"
#include "ab/util/WindowsHelper.hpp"
#include "ui/FlexLayout.hpp"
#include "ui/SplitContainer.hpp"
#include "ui/Tab.hpp"
#include "util/Resources.hpp"

#include "ab/FlatButton.hpp"

namespace chatterino::ui
{
    Window::Window(Application& app, WindowType type)
        : ab::BaseWindow(Flags(Flags::EnableCustomFrame))
        , theme_(this)
        , app_(app)
    {
        // add layout
        this->initLayout();

        // set title
        if (type == WindowType::Main)
            this->setWindowTitle(QString("Chatterino β ") + version);

        // update theme when dpi changes
        QObject::connect(this, &ab::BaseWindow::scaleChanged, this,
            [this]() { this->theme_.updateStyle(); });

        // titlebar
        if (this->hasCustomWindowFrame())
        {
            this->addTitleBarButton(
                ab::makeWidget<ab::FlatButton>([](ab::FlatButton* w) {
                    w->setPixmap(resources().buttons.settings);
                    w->setObjectName("title-bar-custom");
                }));

            this->addTitleBarButton(
                ab::makeWidget<ab::FlatButton>([](ab::FlatButton* w) {
                    w->setPixmap(resources().buttons.user);
                    w->setObjectName("title-bar-avatar");
                }));
        }
        else
        {
            // TODO: add account button
        }
    }

    ThemexD& Window::theme()
    {
        return this->theme_;
    }

    void Window::initLayout()
    {
        // FOURTF: wip
        this->left_ = std::unique_ptr<ab::Notebook>(
            ab::makeWidget<ab::Notebook>([&](auto x) {
                x->setObjectName("left");
                x->setOrientation(ab::NotebookOrientation::Left);
            }));

        // FOURTF: wip
        this->right_ = std::unique_ptr<ab::Notebook>(
            ab::makeWidget<ab::Notebook>([&](auto x) {
                x->setObjectName("right");
                x->setOrientation(ab::NotebookOrientation::Right);
                x->setUndockHandler([](QWidget* tab, QWidget* content) {});

                auto addButton = new ab::NotebookButton();
                QObject::connect(
                    addButton, &ab::FlatButton::leftClicked, this, [this, x]() {
                        auto tab = new Tab("lmao");
                        x->addTab(tab, new SplitContainer(this->app_));
                        x->select(tab);
                    });
                x->addTabAtEnd(addButton);
            }));

        // center widget (tabs)
        this->setCenterWidget(ab::makeWidget<QFrame>([&](auto x) {
            x->setObjectName("content");
            x->setLayout(ab::makeLayout<FlexLayout>(
                [&](auto x) { this->layout_ = x; },
                {
                    ab::makeWidget<ab::Notebook>([&](auto x) {
                        new Tab("asdf");
                        this->center_ = x;
                        x->addTab(
                            new Tab("Tab1"), new SplitContainer(this->app_));

                        auto addButton = new ab::NotebookButton();
                        QObject::connect(addButton,
                            &ab::FlatButton::leftClicked, this, [this, x]() {
                                auto tab = new Tab("lmao");
                                x->addTab(tab, new SplitContainer(this->app_));
                                x->select(tab);
                            });
                        x->addTabAtEnd(addButton);
                    }),
                }));
        }));

        // misc
        this->setAcceptDrops(true);
        this->updateLayout();
    }

    void Window::updateLayout()
    {
        //        if (this->left_)
        //            this->setLeftVisible(this->dragging_);

        //        if (this->right_)
        //            this->setRightVisible(this->dragging_);

        // hide the left and right notebook for now
        this->setLeftVisible(false);
        this->setRightVisible(false);
    }

    void Window::setLeftVisible(bool value)
    {
        // add to layout
        if (value && this->layout_->indexOf(this->left_.get()) == -1)
        {
            this->layout_->addWidgetRelativeTo(
                this->left_.get(), this->center_, Direction::Left);
        }

        // -or- remove from layout
        if (!value && this->layout_->indexOf(this->left_.get()) != -1)
        {
            auto item = this->layout_->takeAt(
                this->layout_->indexOf(this->left_.get()));
            item->widget()->hide();
            delete item;
        }
    }

    void Window::setRightVisible(bool value)
    {
        // add to layout
        if (value && this->layout_->indexOf(this->right_.get()) == -1)
        {
            this->layout_->addWidgetRelativeTo(
                this->right_.get(), this->center_, Direction::Right);
        }

        // -or- remove from layout
        if (!value && this->layout_->indexOf(this->right_.get()) != -1)
        {
            auto item = this->layout_->takeAt(
                this->layout_->indexOf(this->right_.get()));
            item->widget()->hide();
            delete item;
        }
    }

    void Window::drag()
    {
#ifdef USEWINSDK
        // move to cursor position every millisecond
        auto timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, this, [this, timer]() {
            if (this->isVisible())
            {
                if (ab::isLButtonDown())
                    this->move(QCursor::pos() - QPoint(50, 8));
                else
                    timer->stop();
            }
        });
        timer->setInterval(1);
        timer->start();
#endif
    }

    // EVENTS
    void Window::closeEvent(QCloseEvent*)
    {
        // close app if main window
        if (this->type_ == WindowType::Main)
        {
            QApplication::exit();
        }
    }

    void Window::showEvent(QShowEvent*)
    {
        // this->themeChangedEvent(this->initialTheme_);
    }

    void Window::dragEnterEvent(QDragEnterEvent* event)
    {
        event->accept();

        this->dragging_ = true;
        this->updateLayout();
    }

    void Window::dragLeaveEvent(QDragLeaveEvent* event)
    {
        event->accept();

        this->dragging_ = false;
        this->updateLayout();
    }
}  // namespace chatterino::ui
