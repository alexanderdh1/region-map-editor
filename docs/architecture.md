# Architecture
This document describes the software architecture of the Spatial Map Editor. The goal of the architecture is to keep the system understandable, modular, and easy to extend, while remaining close to what is actually implemented in code.

The architecture is intentionally kept simple and explicit. Every part of the system has a clear responsibility, and communication between parts is tightly controlled.

## 1. Architectural Goals

The architecture is designed to achieve the following:

- Clear separation of responsibilities

- Predictable data flow

- Low coupling between subsystems

- Easy extension without breaking existing code

- Good performance for large spatial datasets

The system favors clarity over abstraction. If a concept is hard to explain, it does not belong in the architecture.

## 2. System Overview
### 2.1 Overall purpose of the application

The Spatial Map Editor is a desktop application designed for visualizing and interacting with large-scale spatial data. The system allows users to navigate, inspect, and organize extensive map-like worlds using precise coordinate-based interaction.

The primary use case of the application is as a development and planning tool, where large areas can be explored, divided into regions, and thoroughly planned. The system is suitable for working with abstract maps, fictional worlds, and other large spatial datasets where structure and overview are more important than real-time simulation.

The application is designed to be flexible and easy to adapt, focusing on giving the user clear control over their custom map rather than enforcing domain-specific rules and restrictions.

### 2.2 High-level data flow through the system

This section is intentionally left undefined at this stage.  
The concrete data flow will be documented once core systems such as camera handling, input processing, and rendering have been implemented.

### 2.3 Intended usage and system type

The application is intended to be used as an offline planning and visualization tool rather than a real-time simulation or game engine. It is designed for users who need to explore, structure, and manage large maps or worlds.

Typical usage involves loading a spatial image source, navigating the map through zooming and panning, and defining regions that represent user-defined areas of interest. These regions can then be annotated, organized hierarchically, and used to track progress and store notes.

The system is designed as a single-user desktop tool, prioritizing clarity, precision, and control over automation. It is not intended for collaborative editing, real-time synchronization, or live data streaming.


### 2.4 Decomposition into smaller modules

To keep the system understandable and maintainable, the application is decomposed into a small set of distinct modules, each responsible for a specific part of the system. This separation ensures that functionality is grouped by responsibility.

The system is divided into the following modules:

- Core – coordinates application flow and system logic
- Data – owns and manages all persistent application state
- Input – translates user input into actions
- Rendering – visualizes the current application state
- UI – graphical front-end for user interaction and inspection

```
             +--------+
             |  User  |
             +--------+
                 |
                 v
           +-------------+
           |   Input     |
           +-------------+
                 |
                 v
+-------+    +-------------+    +-----------+
|  UI   | <- |    Core     | -> | Rendering |
+-------+    +-------------+    +-----------+
                 |
                 v
           +-------------+
           |    Data     |
           +-------------+
```
This diagram shows relationships between modules rather than runtime communication.

These modules form the structural foundation of the application and are described in more detail in the following section.

## 3. Module Breakdown
For each module we will describe the following:
- What is the modules function and what is it responsible for?
- How does it fit into the overall system?
### 3.1 Core
The Core module is the central coordinator of the system. It is responsible for controlling the overall application flow and for making decisions based on user actions and the current system state.

The primary responsibilities of the Core module include:
- Managing the application lifecycle, such as startup and shutdown
- Coordinating the main update loop
- Holding the current application state and tracking what the user is currently doing
- Dispatching actions to subsystems and coordinating interaction between modules

The Core is the only module allowed to coordinate interactions between other modules, although it does not implement or execute their internal logic. The module mainly provides structure to the system. By centralizing decision-making, it ensures that the overall program behaves as intended and that system behavior is easier to reason about and debug.


### 3.2 Data
The Data module is responsible for owning and managing the current application state. It defines how spatial data, regions, and associated metadata are stored, organized, and accessed.

The primary responsibilities of the Data module include:
- Storing and managing spatial data representing the map or world
- Representing regions and their geometric definitions
- Managing hierarchical relationships between regions
- Storing metadata such as notes and other region-related information
- Providing controlled access to the current system state

The Data module acts as the primary source of information in the system. All modifications to persistent state are performed through the Core module, ensuring that state changes remain predictable for all other modules and that the integrity of the data model is preserved.

### 3.3 Input
The Input module is responsible for handling raw user input and translating it into meaningful actions that the rest of the system can respond to. It acts as an abstraction layer between physical user input and application logic.

The primary responsibilities of the Input module include:
- Handling mouse and keyboard input
- Interpreting user actions such as clicks, dragging, and scrolling
- Translating raw input into actions such as navigation, selection, or editing
- Mapping screen-space input to world-space coordinates when required

The Input module does not directly modify application state. Instead, it communicates user intent to the Core module, which decides how the system should respond. This separation ensures that input handling remains flexible and that changes to interaction methods do not affect core system logic.

### 3.4 Rendering
The Rendering module is responsible for visualizing the current application state. It translates spatial data and camera information into graphical output that represents the map, regions, and other visual elements on screen.

The primary responsibilities of the Rendering module include:
- Rendering the spatial image source (e.g. map or tiled data)
- Applying camera transformations such as position and zoom
- Converting world-space coordinates into screen-space output
- Drawing regions, overlays, and other visual indicators
- Ensuring efficient rendering of large spatial datasets

The Rendering module operates purely on the current system state provided by the Core and Data modules. It does not make decisions, interpret user input, or modify application state. By keeping rendering focused solely on visualization, the system remains predictable and easier to optimize and extend.

### 3.5 UI
The UI module provides the graphical user interface through which the user inspects and interacts with the system. It presents information to the user and offers controls for editing and managing data in a structured and accessible way.

The primary responsibilities of the UI module include:
- Displaying panels, overlays, and interface elements
- Presenting metadata such as notes, progress, and status information
- Providing controls for editing region-related data
- Reflecting the current application state in a clear and user-friendly manner

The UI module does not directly modify application state or control system logic. Instead, it communicates user actions to the Core module, which determines how those actions affect the system. This separation ensures that the UI remains flexible and that changes to the interface do not impact core application behavior.

## 4. General Data & State Information
The system maintains a shared application state that represents the current configuration of the map, user interaction, and relevant metadata. This state defines how the system behaves and what is presented to the user at any given time.

Application state includes, but is not limited to:
- Camera parameters such as position and zoom level
- Spatial data representing the map or world
- Region definitions and their hierarchical relationships
- Metadata associated with regions, such as notes and progress
- Information about current user interaction (e.g. selected region or active tool)

State ownership is clearly defined within the system. Persistent state is owned by the Data module, while high-level application state and interaction state are coordinated by the Core module. Other modules may read state as needed, but state changes are performed in a controlled manner to ensure consistency and predictability.

## 5. Data Flow
At a conceptual level, the system follows a predictable flow from user interaction to visual output. User input is interpreted as high-level actions, which are processed by the Core module to update application state. The current state is then visualized through the Rendering module, with the UI reflecting relevant information and available actions.

Rather than relying on implicit behavior or tightly coupled modules, the system enforces a clear separation between input handling, decision-making, state management, and visualization. This approach ensures that system behavior remains understandable and that changes in one part of the system do not unintentionally affect others.

This section describes architectural intent rather than runtime execution order and may be refined as the system evolves.

## 6. Scope & Expansions
The system is designed as a single-user, offline application focused on planning, visualization, and organization of large spatial datasets. It prioritizes clarity, control, and flexibility over automation or real-time collaboration.

The following features are explicitly considered out of scope for the current system:
- Multiplayer or collaborative editing
- Real-time synchronization or networking
- Live data streaming or external data dependencies
- Simulation or game-engine behavior

Potential future expansions may include quality-of-life improvements such as additional interaction tools, improved visualization options, or extended metadata support. These expansions are expected to build on the existing architecture without requiring fundamental structural changes.