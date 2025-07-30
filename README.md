# NetBlocker: A Targeted Network Control Utility

NetBlocker is a specialized Windows application designed to provide granular control over an individual program's network access. Unlike system-wide internet disconnections, NetBlocker operates by dynamically managing Windows Firewall rules, allowing you to selectively block or unblock internet connectivity for a chosen application. This capability can be particularly useful in scenarios where you need to temporarily isolate an application from the network, such as when testing offline functionalities, preventing unwanted updates, or, in some specific contexts, analyzing application behavior under network instability—often referred to as a "lag switch" in gaming or network testing environments.

The application allows users to specify the executable name of the target program and provides a configurable hotkey for instantaneous toggling of its internet access. To enhance usability, NetBlocker includes audible notifications for state changes (blocked/unblocked) and persists all user settings, including the chosen application, hotkey assignment, and UI aesthetic preferences, in a `config.json` file. The user interface is built using ImGui and rendered with DirectX 11, ensuring a lightweight and responsive experience.

## Features

* **Per-Application Network Control:** Precisely block or unblock internet access for a single, specified application without affecting other running programs or the operating system's overall connectivity.
* **Hotkey Toggle:** Rapidly switch the network state of the attached application using a user-defined hotkey. This allows for quick on-the-fly adjustments without interacting directly with the GUI.
* **Configurable Sound Notifications:** Receive auditory feedback when the application's network state changes, with support for custom `.wav` sound files to personalize notifications.
* **Persistent Settings:** All configurations, including the target process, hotkey, and UI color scheme, are automatically saved and loaded from `config.json`, maintaining your preferences across sessions.
* **Lightweight UI:** Utilizes ImGui and DirectX 11 for an efficient and non-intrusive graphical user interface.

## How it Works (The "Lag Switch" Mechanism)

NetBlocker functions by interacting directly with the Windows Firewall API. When you initiate a "block" action for an application, NetBlocker programmatically adds a new outbound firewall rule that specifically denies network access for the executable path of the attached process. Conversely, when you "unblock" it, NetBlocker removes this specific firewall rule.

This rapid creation and deletion of firewall rules can be leveraged to achieve an effect similar to a "lag switch." By quickly toggling the network state, you can briefly disconnect an application from its network resources. This can be particularly useful for:

* **Debugging Network Behavior:** Observing and analyzing how an application responds to temporary network interruptions or disconnections.
* **Testing Offline Modes:** Verifying that an application gracefully handles a sudden loss of internet connectivity and transitions correctly into an offline operational mode.
* **Simulating Network Instability:** For developers or testers, this can simulate intermittent network issues, allowing for testing of an application's resilience and error handling under adverse network conditions.
* **Specific Gaming Scenarios (Use with Caution):** In certain peer-to-peer online games, temporarily cutting off your outbound connection can pause your game state for other players while your client continues to process inputs. This leads to a temporary desynchronization, which some users might exploit for various reasons. **Users should be aware that such practices may violate the terms of service of online games and could lead to penalties.**

It's important to understand that NetBlocker does not introduce actual network latency; instead, it creates a complete (though temporary) disconnection. The perceived "lag" or "pause" is a byproduct of the application attempting to reconnect or reconcile its state with the sudden loss of network resources.

## Usage

1.  **Visibility Toggle:** Press the `INSERT` key to show or hide the NetBlocker main interface.
2.  **Target Application:** In the NetBlocker UI, enter the exact executable name of the application you wish to control (e.g., `RobloxPlayerBeta.exe`).
3.  **Attach Process:** Click the "Attach" button to link NetBlocker to the specified application. This action identifies the running instance of the program and prepares it for network management.
4.  **Toggle Network Access:** Utilize the configured hotkey (default is `F1`) to instantly toggle the internet connectivity for the attached application.
5.  **Customization (Optional):**
    * **UI Color:** Modify the `accentColor` values within the `config.json` file to personalize the user interface's primary color.
    * **Sound Packs:** Create subdirectories within the `sounds/` folder (e.g., `sounds/my_pack/`) and place `on.wav` and `off.wav` files inside to create custom sound notification packs.

## Contributing

Contributions are welcome! If you have suggestions for improvements, bug reports, or would like to contribute code, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/mit-license.php). See the `LICENSE` file for full details.