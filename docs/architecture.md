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
- Responsibilities and boundaries of each module
### 3.1 Core

### 3.2 Data

### 3.3 Input

### 3.4 Rendering

## 4. General Data & State Information

- What data is considered part of the system state

- Ownership and rules for updating state

## 5. Data Flow

- The flow from user input to visual output

- The order in which subsystems are executed

## 6. Scope & Expansions

- What the system is designed to do

- What is explicitly out of scope

- Possible future quality-of-life extensions
