https://smart-pill-box--nothingtosay257.replit.app
# Web Dashboard

This folder contains the Replit-built web dashboard for the **Smart Medication Reminder Pill Box** project.

## Overview

The web dashboard is used to monitor and manage the smart pill box remotely. It connects with Firebase Realtime Database to display medicine schedules, dose status, pill inventory, logs, and device information.

## Features

* View morning, afternoon, and night medicine schedules
* Update dose timings remotely
* Monitor Taken, Missed, and Pending status
* Display remaining pill inventory
* Show dose logs from Firebase
* Support real-time dashboard updates
* Provide cloud-side visibility for caregivers

## Technology Used

* Replit
* HTML / CSS / JavaScript
* Firebase Realtime Database
* Web dashboard UI
* Python gateway integration

## Folder Contents

```text
app/
├── app-code.zip
├── screenshots/
└── README.md
```

## Source Code

The complete Replit dashboard source code is included as:

```text
app-code.zip
```

The zip file contains the cleaned app source code without unnecessary dependency folders like `node_modules`.

## Live Demo

Temporary Replit deployment link:

```text
PASTE_YOUR_REPLIT_LINK_HERE
```

Note: This Replit deployment link may expire because it is hosted on a temporary/free deployment.

## Screenshots

Dashboard screenshots are stored inside:

```text
screenshots/
```

Recommended screenshots:

* Dashboard home page
* Schedule update page
* Inventory page
* Logs page
* Firebase connection/status view

## Role in Project

The web dashboard acts as the cloud interface of the system. A caregiver can view medicine status, update schedules, and monitor whether the patient has taken or missed a dose.

The Arduino hardware handles the reminder locally, while the dashboard provides remote monitoring through Firebase.

## Future Improvements

* Host dashboard permanently using Firebase Hosting or Vercel
* Add login authentication
* Add mobile-friendly UI
* Add SMS or WhatsApp alert integration
* Add charts for medication history
