.. zephyr:code-sample:: btreefy-door-controller
   :name: BTreeFy Door Controller
   :relevant-api: gpio_interface

   Application showing the execution of a Behavior Tree to control a door system.

Overview
********

This example application demonstrates how behavior trees can be used to model an embedded system project using the BTreeFy library.

The application simulates a door controller that manages a motor to open and close a door based on user requests and safety conditions.

The source code shows how to:

#. Use BTreeFy to execute a behavior tree in a Zephyr environment.
#. Use a single button with multi-press logic to trigger different events (Emergency, Open Request, Close Request).
#. Simulate asynchronous hardware behavior (door movement) using Zephyr timers.
#. Use a blackboard to share state between the behavior tree and the application logic.

User Interaction
****************

The system uses a single button (``sw0``) to receive user commands:

- **1 Press**: Triggers an **Emergency** routine. The door will stop immediately and then open.
- **2 Presses**: Triggers an **Open Request**.
- **3 Presses**: Triggers a **Close Request**.

The status of the door and motor is printed to the console.

Requirements
************

Your board must:

#. Have an LED connected via a GPIO pin (defined as ``led0``).
#. Have a Button connected via a GPIO pin (defined as ``sw0``).

Building and Running
********************

Build and flash the application as follows, using ``native_posix`` as an example:

.. zephyr-app-commands::
   :zephyr-app: examples/zephyr-app/behavior-tree-app/app
   :board: native_posix
   :goals: build run
   :compact:

Expected Output
***************

When running the application, you will see logs indicating the button presses and the behavior tree's response. For example:

.. code-block:: none

   Set up button at GPIO_0 pin 1
   Button timer callback executed
   button_pressed_count = 2
   Open request on timer callback
   Door is opening...  request ACCEPTED!
   Door sensor timer callback
   Door sensor timer callback
   Door is open
