#include "stdafx.h"
#include "InputMapper.h"

#include <SDL\SDL_keyboard.h>
#include <SDL\SDL_mouse.h>
#include <Vorb/io/Keg.h>

#include "global.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Inputs.h"

#include "VirtualKeyKegDef.inl"

struct InputKegArray {
    Array<VirtualKey> defaultKey;
    Array<VirtualKey> key;
};
KEG_TYPE_DECL(InputKegArray);
KEG_TYPE_DEF(InputKegArray, InputKegArray, kt) {
    using namespace keg;
    kt.addValue("defaultKey", Value::array(offsetof(InputKegArray, defaultKey), Value::custom(0, "VirtualKey", true)));
    kt.addValue("key", Value::array(offsetof(InputKegArray, key), Value::custom(0, "VirtualKey", true)));
}
InputMapper::InputMapper() {
    // Empty
}

InputMapper::~InputMapper() {
    for (unsigned int i = 0; i < m_inputs.size(); i++) {
        delete m_inputs[i];
    }
}

bool InputMapper::getInputState(const InputMapper::InputID id) {
    // Check Input
    if (id < 0 || id >= m_inputs.size()) return false;

    Input* input = m_inputs.at(id);
    return m_keyStates[input->key];
}

InputMapper::InputID InputMapper::createInput(const nString& inputName, VirtualKey defaultKey) {
    InputMapper::InputID id = getInputID(inputName);
    if (id >= 0) return id;
    id = m_inputs.size();
    m_inputLookup[inputName] = id;
    Input *input = new Input();
    input->name = inputName;
    input->defaultKey = defaultKey;
    input->key = defaultKey;
    input->upEvent = Event<ui32>(this);
    input->downEvent = Event<ui32>(this);
    m_inputs.push_back(input);
    return id;
}

InputMapper::InputID InputMapper::getInputID(const nString& inputName) const {
    auto iter = m_inputLookup.find(inputName);

    if (iter != m_inputLookup.end()) {
        return iter->second;
    } else {
        return -1;
    }
}

void InputMapper::loadInputs(const std::string &location /* = INPUTMAPPER_DEFAULT_CONFIG_LOCATION */) {
    vio::IOManager ioManager; //TODO PASS IN
    nString data;
    ioManager.readFileToString(location.c_str(), data);
   
    if (data.length() == 0) {
        fprintf(stderr, "Failed to load %s", location.c_str());
        throw 33;
    }

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        perror(location.c_str());
        reader.dispose();
        throw 34;
    }

    // Manually parse yml file
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&] (Sender, const nString& name, keg::Node value) {
        Input* curInput = new Input();
        curInput->name = name;
        keg::parse((ui8*)curInput, value, reader, keg::getGlobalEnvironment(), &KEG_GET_TYPE(Input));
        
        curInput->upEvent = Event<ui32>(curInput);
        curInput->downEvent = Event<ui32>(curInput);

        m_inputLookup[curInput->name] = m_inputs.size();
        m_inputs.push_back(curInput);
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();
}

void InputMapper::startInput() {
    vui::InputDispatcher::mouse.onButtonDown += makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp += makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &InputMapper::onKeyDown);
    vui::InputDispatcher::key.onKeyUp += makeDelegate(*this, &InputMapper::onKeyUp);
}
void InputMapper::stopInput() {
    vui::InputDispatcher::mouse.onButtonDown -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &InputMapper::onKeyDown);
    vui::InputDispatcher::key.onKeyUp -= makeDelegate(*this, &InputMapper::onKeyUp);
}

InputMapper::Listener* InputMapper::subscribe(const i32 inputID, EventType eventType, InputMapper::Listener* f) {
    if(inputID < 0 || inputID >= m_inputs.size() || f == nullptr || m_inputs[inputID]->type != InputType::SINGLE_KEY) return nullptr;
    switch (eventType) {
    case UP:
        m_inputs[inputID]->upEvent.add(*f);
        return f;
    case DOWN:
        m_inputs[inputID]->downEvent.add(*f);
        return f;
    }
    return nullptr;
}

void InputMapper::unsubscribe(const i32 inputID, EventType eventType, InputMapper::Listener* f) {
    if(inputID < 0 || inputID >= m_inputs.size() || f == nullptr || m_inputs[inputID]->type != InputType::SINGLE_KEY) return;
    switch(eventType) {
    case UP:
        m_inputs[inputID]->upEvent.remove(*f);
    case DOWN:
        m_inputs[inputID]->downEvent.remove(*f);
    }
}

void InputMapper::saveInputs(const nString &filePath /* = DEFAULT_CONFIG_LOCATION */) {
    //TODO(Ben): Implement
}

VirtualKey InputMapper::getKey(const i32 inputID) {
    if (inputID < 0 || inputID >= m_inputs.size()) return VKEY_HIGHEST_VALUE;
    return m_inputs.at(inputID)->key;
}

void InputMapper::setKey(const i32 inputID, VirtualKey key) {
    m_inputs.at(inputID)->key = key;
}

void InputMapper::setKeyToDefault(const i32 inputID) {
    if(inputID < 0 || inputID >= m_inputs.size()) return;
    Input* input = m_inputs.at(inputID);
    input->key = input->defaultKey;
}

void InputMapper::onMouseButtonDown(Sender, const vui::MouseButtonEvent& e) {
    switch (e.button) {
    case vui::MouseButton::LEFT:
        m_keyStates[SDL_BUTTON_LEFT] = true;
        break;
    case vui::MouseButton::RIGHT:
        m_keyStates[SDL_BUTTON_RIGHT] = true;
        break;
    default:
        break;
    }
}
void InputMapper::onMouseButtonUp(Sender, const vui::MouseButtonEvent& e) {
    switch (e.button) {
    case vui::MouseButton::LEFT:
        m_keyStates[SDL_BUTTON_LEFT] = false;
        break;
    case vui::MouseButton::RIGHT:
        m_keyStates[SDL_BUTTON_RIGHT] = false;
        break;
    default:
        break;
    }
}
void InputMapper::onKeyDown(Sender, const vui::KeyEvent& e) {
    m_keyStates[e.keyCode] = true;
}
void InputMapper::onKeyUp(Sender, const vui::KeyEvent& e) {
    m_keyStates[e.keyCode] = false;
}
