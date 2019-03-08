#include "Application.hpp"

#include <stdlib.h>
#include <QApplication>
#include <QVector>

#include "net/NetworkManager.hpp"
#include "twitch/TwitchProvider.hpp"
#include "ui/Window.hpp"

#include "controllers/accounts/AccountController.hpp"
#include "controllers/commands/CommandController.hpp"
#include "controllers/highlights/HighlightController.hpp"
#include "controllers/ignores/IgnoreController.hpp"
#include "controllers/moderationactions/ModerationActions.hpp"
#include "controllers/notifications/NotificationController.hpp"
#include "controllers/taggedusers/TaggedUsersController.hpp"
#include "messages/MessageBuilder.hpp"
#include "providers/bttv/BttvEmotes.hpp"
#include "providers/chatterino/ChatterinoBadges.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/twitch/PubsubClient.hpp"
//#include "providers/twitch/TwitchServer.hpp"
#include "singletons/Emotes.hpp"
#include "singletons/Logging.hpp"
#include "singletons/NativeMessaging.hpp"
#include "singletons/Paths.hpp"
#include "singletons/Settings.hpp"
#include "singletons/Toasts.hpp"
//#include "singletons/WindowManager.hpp"
//#include "util/IsBigEndian.hpp"
//#include "util/Log.hpp"
//#include "util/PostToThread.hpp"

namespace chatterino
{
    class PrivateApplication
    {
    public:
        PrivateApplication(/*QApplication& qtApp*/)
        //            : qtApp(qtApp)
        {
            // need to pass the reference via the constructor
        }

        // windows
        ui::Window* mainWindow{};
        QVector<ui::Window*> windows;

        // providers
        QVector<Provider*> providers;

        // misc
        QObject object;
        // QApplication& qtApp;
    };

    Application::Application(/*QApplication& qtApp*/)
        // leaking (compatability)
        : toasts(new Toasts())
        , emotes(new Emotes())
        //, windows(new WindowManager())

        , accounts(new AccountController())
        //, commands(new CommandController())
        , highlights(new HighlightController())
        , notifications(new NotificationController())
        , ignores(new IgnoreController())
        , taggedUsers(new TaggedUsersController())
        , moderationActions(new ModerationActions())
        , chatterinoBadges(new ChatterinoBadges())
        , logging(new Logging())

        , this_(new PrivateApplication(/*qtApp*/))
    {
        appInst__ = this;

        this_->providers.append(new TwitchProvider());

        NetworkManager::init();
    }

    Application::~Application()
    {
        _exit(0);

        delete this_;
    }

    void Application::run(QApplication& qtApp)
    {
        // make sure that there is a main window
        if (!this_->mainWindow)
            this_->mainWindow = addWindow(ui::WindowType::Main);

        // run the main event loop
        qtApp.exec();
    }

    ui::Window* Application::addWindow(const ui::WindowType& type)
    {
        // make new window
        auto window = new ui::Window(*this, type);

        // add to list
        this_->windows.append(window);

        // remove from list when destroyed
        QObject::connect(window, &QWidget::destroyed, &this_->object,
            [=]() { this_->windows.removeOne(window); });

        // show
        window->show();

        return window;
    }

    const QVector<Provider*>& Application::providers()
    {
        return this_->providers;
    }

    void Application::alert()
    {
        int duration = getSettings()->longAlerts ? 2500 : 0;

        if (this_->mainWindow)
            QApplication::alert(this_->mainWindow->window(), duration);
    }

    void Application::initialize(Settings& settings, Paths& paths)
    {
        // compatability
        this->toasts->initialize(settings, paths);
        this->emotes->initialize(settings, paths);
        // WindowManager* const windows{};

        this->accounts->initialize(settings, paths);
        // this->commands->initialize(settings, paths);
        this->highlights->initialize(settings, paths);
        this->notifications->initialize(settings, paths);
        this->ignores->initialize(settings, paths);
        this->taggedUsers->initialize(settings, paths);
        this->moderationActions->initialize(settings, paths);
        this->chatterinoBadges->initialize(settings, paths);

        this->logging->initialize(settings, paths);
    }

    Application* getApp()
    {
        assert(appInst__);

        return appInst__;
    }
}  // namespace chatterino
