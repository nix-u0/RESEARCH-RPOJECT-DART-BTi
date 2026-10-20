# Project BTI Drone

Project BTI Drone is a browser-based ground station for controlling an Arduino
servo and monitoring a drone sprayer workflow. The app provides manual servo
angle controls, arming safety, spray timing controls, telemetry logs, camera
preview, and optional peer-to-peer video and control synchronization.

## Features

- Connects a browser to an Arduino over USB.
- Supports native Web Serial where available.
- Supports Android Pixel devices through WebUSB.
- Includes a direct WebUSB driver for common CH340/CH341 adapters.
- Sends servo angle commands at `9600 baud`.
- Provides angle presets for `0`, `90`, and `180` degrees.
- Provides camera capture and PeerJS WebRTC video/data sharing.
- Displays connection, transmission, and application telemetry.

The current Arduino sketch controls the servo connected to pin 9. **SPRAY NOW**
sends `180` to open the servo, waits for the selected duration, and sends `0`
to return it to the closed position. Pump or spray-output control must be added
separately if required.

## Hardware

- Arduino UNO or compatible board.
- Servo connected to digital pin 9.
- USB cable and, for a Pixel, a USB-C OTG data adapter.
- CH340/CH341 USB adapter, or a CDC-ACM-compatible USB serial interface.
- Pixel USB host mode and USB permission for Android testing.

## Android and CH340 setup

Use Chrome on the Pixel and serve the page over HTTPS. Connect the Arduino
with a data-capable OTG adapter, press **Connect USB**, select the CH340 device,
and approve the Android USB permission prompt.

The direct CH340 driver supports common product IDs including `0x7523`,
`0x7522`, and `0x5523`. A different clone may require additional protocol
support. If the device does not appear in Chrome's USB chooser, verify that
Android detects the device and try another OTG adapter or data cable.

## Development Tools

### Application

- HTML5, CSS3, and vanilla JavaScript.
- Tailwind CSS via CDN for utility styling.
- Lucide Icons via CDN for interface icons.
- PeerJS `1.5.2` for WebRTC data and media connections.
- Web Serial API for browsers with native serial support.
- WICG Web Serial Polyfill `1.0.15` for CDC-ACM devices over WebUSB.
- WebUSB API for the direct CH340/CH341 transport on Android.
- Browser `MediaDevices`, `MediaStream`, `Canvas`, and `TextEncoderStream` APIs.
- Firebase Storage and Firestore for optional cloud capture uploads.

### Embedded hardware

- Arduino IDE for compiling and uploading the sketch.
- Arduino C++.
- `Servo` library for servo positioning.

### Development environment

- Visual Studio Code.
- Google Chrome or Chromium-based browser.
- HTTPS development hosting for WebUSB and camera permissions.
- USB-C OTG adapter and data cable for Pixel testing.

## Project Structure

```text
projectBTI-drone/
|-- index.html             Browser ground station
|-- README.md              Project documentation
`-- arduino/
    `-- arduino.ino        Arduino servo controller
```

## Serial Protocol

The browser sends newline-terminated commands:

```text
0
90
180
```

The Arduino sketch accepts numeric commands and moves the servo between `0` and
`180` degrees. The browser handles the timed open-and-close sequence.

## Teachable Machine scene workflow

The web app can load an image model exported by Google Teachable Machine. In
the **AI Scene Analysis** panel:

1. Publish or host the Teachable Machine model and paste its model URL. The
    URL should end with a model directory containing `model.json` and
    `metadata.json`.
2. Use scene classes that match the capture labels: `Stagnant Water`,
    `Containers`, `Tires`, and `Other`.
3. Set a conservative confidence threshold. Start with `0.90` and lower it
    only after reviewing false positives and false negatives.
4. Load the model and use **Analyze Frame** to test individual aerial frames.
5. Choose a decision mode. Start with **Human Only**, then use **AI Suggestion
    + Confirm** for supervised testing. Enable **AI Auto-Trigger** only after
    validating the model on images from the actual drone camera, altitude,
    lighting, and motion conditions.

Captured images are stored in the browser's IndexedDB database and remain
available after a page reload on the same device and browser. The table records
the image, capture time, predicted class, and confidence. Individual images can
be downloaded for labeling and dataset review.

### Optional Firebase uploads

The capture table includes per-image upload buttons and an **Upload All**
action. Uploading stores the PNG in Firebase Storage under
`training-captures/<label>/` and stores metadata in the Firestore
`trainingCaptures` collection. Firebase is optional; IndexedDB remains the
local fallback.

To enable it:

1. Create a Firebase project with Storage and Firestore enabled.
2. Register a web app and copy its Firebase web configuration.
3. Define `window.DART_FIREBASE_CONFIG` before the main application script in
   `index.html`, using the configuration from Firebase Console:

```html
<script>
    window.DART_FIREBASE_CONFIG = {
        apiKey: 'your-api-key',
        authDomain: 'your-project.firebaseapp.com',
        projectId: 'your-project-id',
        storageBucket: 'your-project.firebasestorage.app',
        messagingSenderId: 'your-sender-id',
        appId: 'your-app-id'
    };
</script>
```

4. Configure Firebase Authentication and security rules before field use. Do
   not make Storage or Firestore publicly writable; require an authenticated
   user and validate allowed labels and file types in the rules.
5. Reload the app. The capture panel should show **Firebase ready**.

Firebase web configuration values are identifiers, not passwords, but access
must still be protected with Authentication and rules. Teachable Machine does
not automatically receive uploads; review the Firebase images and import the
selected labeled images into Teachable Machine for training.

Use the **Dataset label** selector before pressing **Capture Photo**. Useful
examples include `Stagnant Water`, `Containers`, `Tires`, and `Other`,
and `Other`. These labels describe the scene or training category and are
separate from the AI model's configured target class label. Download the images and
place them into matching folders when preparing the Teachable Machine dataset.

## Detection and motor safety

The **Decision mode** control provides three operating modes:

- **Human Only:** AI can be loaded and analyzed manually, but it cannot trigger
    the motor. The operator decides when to press **SPRAY NOW**.
- **AI Suggestion + Confirm:** a qualifying AI result displays a trigger
    suggestion. The motor remains stopped until the operator presses
    **Confirm Trigger**.
- **AI Auto-Trigger:** a qualifying AI result may trigger the motor
    automatically after all safety checks pass.

AI-triggered motor activation requires all of these conditions:

- The Teachable Machine model is loaded.
- The predicted class is `Stagnant Water`, `Containers`, or `Tires`.
- Confidence is at or above the configured threshold.
- **AI Suggestion + Confirm** or **AI Auto-Trigger** is selected.
- The system is explicitly armed.
- The Arduino USB connection is active.
- No spray cycle is currently running.

For field testing, use **Human Only** first, then **AI Suggestion + Confirm**.
Keep **AI Auto-Trigger** disabled until you have reviewed false positives,
false negatives, model confidence, camera motion blur, altitude, and lighting.

The active spray duration is captured when **SPRAY NOW** starts. Changing the
slider does not alter an active cycle. **CLOSE NOW** and disarming immediately
send the closed angle to the servo and cancel the active timer.

For training, collect balanced examples of target and non-target scenes,
including water, vegetation, shadows, reflections, empty targets, different
altitudes, and motion blur. Keep a validation set that is not used during
training. Do not rely on a single prediction for safety-critical actuation;
use a review or confirmation workflow until field testing demonstrates reliable
