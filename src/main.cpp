// FPS Optimizer - Geode mod
// 1) FPS counter pinned to the top-left corner, visible across every scene
// 2) Optional performance tweaks: particle limiter, decoration hiding
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCParticleSystem.hpp>
#include <Geode/modify/GameObject.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// Cached settings (hooks run very often, so we don't query Mod::get() in them)
// ---------------------------------------------------------------------------
namespace {
    int  g_particleDensity = 100;
    bool g_hideDecorations = false;
}

$on_mod(Loaded) {
    auto mod = Mod::get();
    g_particleDensity = static_cast<int>(mod->getSettingValue<int64_t>("particle-density"));
    g_hideDecorations = mod->getSettingValue<bool>("hide-decorations");

    listenForSettingChanges("particle-density", [](int64_t v) {
        g_particleDensity = static_cast<int>(v);
    });
    listenForSettingChanges("hide-decorations", [](bool v) {
        g_hideDecorations = v;
    });
}

// ---------------------------------------------------------------------------
// FPS counter node (kept alive across scene changes)
// ---------------------------------------------------------------------------
class FpsCounter : public CCNode {
protected:
    CCLabelBMFont* m_label = nullptr;
    float m_elapsed = 0.f;
    int   m_frames  = 0;
    float m_fps     = 0.f;

    bool init() override {
        if (!CCNode::init()) return false;

        m_label = CCLabelBMFont::create("-- FPS", "bigFont.fnt");
        m_label->setAnchorPoint({0.f, 1.f});   // top-left anchored text
        this->addChild(m_label);

        this->setID("fps-counter"_spr);
        this->scheduleUpdate();
        this->refresh();
        return true;
    }

    void refresh() {
        auto mod = Mod::get();

        // text
        int shown = static_cast<int>(std::lround(m_fps));
        if (mod->getSettingValue<bool>("show-frametime") && m_fps > 0.f) {
            m_label->setString(fmt::format("{} FPS ({:.1f} ms)", shown, 1000.f / m_fps).c_str());
        } else {
            m_label->setString(fmt::format("{} FPS", shown).c_str());
        }

        // color
        if (mod->getSettingValue<bool>("color-code")) {
            double interval = CCDirector::get()->getAnimationInterval();
            float target = interval > 0.0 ? static_cast<float>(1.0 / interval) : 60.f;
            if (m_fps >= target * 0.9f)      m_label->setColor({90, 255, 90});
            else if (m_fps >= target * 0.6f) m_label->setColor({255, 220, 60});
            else                             m_label->setColor({255, 70, 70});
        } else {
            m_label->setColor({255, 255, 255});
        }

        // look
        m_label->setScale(static_cast<float>(mod->getSettingValue<double>("counter-scale")));
        m_label->setOpacity(static_cast<GLubyte>(
            255.0 * mod->getSettingValue<double>("counter-opacity")));
    }

    void place() {
        auto win = CCDirector::get()->getWinSize();
        this->setPosition({4.f, win.height - 4.f});   // top-left corner, small margin
    }

    void update(float dt) override {
        auto mod = Mod::get();

        bool visible = mod->getSettingValue<bool>("show-fps");
        if (visible && mod->getSettingValue<bool>("only-in-level")) {
            visible = PlayLayer::get() != nullptr || LevelEditorLayer::get() != nullptr;
        }
        this->setVisible(visible);
        if (!visible) return;

        m_frames++;
        m_elapsed += dt;

        float interval = static_cast<float>(mod->getSettingValue<double>("update-interval"));
        if (m_elapsed >= interval && m_elapsed > 0.f) {
            m_fps = static_cast<float>(m_frames) / m_elapsed;
            m_frames = 0;
            m_elapsed = 0.f;
            this->refresh();
        }
        this->place();
    }

public:
    static FpsCounter* create() {
        auto ret = new FpsCounter();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Create the counter once, the first time the main menu loads.
class $modify(FpsMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        static bool created = false;
        if (!created) {
            created = true;
            auto counter = FpsCounter::create();
            if (counter) SceneManager::get()->keepAcrossScenes(counter);
        }
        return true;
    }
};

// ---------------------------------------------------------------------------
// Optimizer 1: limit particle spawning while inside a level
// ---------------------------------------------------------------------------
class $modify(OptParticleSystem, CCParticleSystem) {
    bool addParticle() {
        if (g_particleDensity < 100 && PlayLayer::get()) {
            if (static_cast<int>(std::rand() % 100) >= g_particleDensity) return false;
        }
        return CCParticleSystem::addParticle();
    }
};

// ---------------------------------------------------------------------------
// Optimizer 2: hide decoration-type objects while playing (off by default)
// ---------------------------------------------------------------------------
class $modify(OptGameObject, GameObject) {
    void setVisible(bool visible) {
        if (visible && g_hideDecorations && m_objectType == GameObjectType::Decoration && PlayLayer::get()) {
            visible = false;
        }
        GameObject::setVisible(visible);
    }
};
