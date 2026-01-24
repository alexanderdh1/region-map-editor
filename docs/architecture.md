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