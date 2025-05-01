The program starts in auto mode which is essentially a function with the following logic.

```mermaid
sequenceDiagram
    loop Every Second
        Code->>DHT22: readTemperature()
        Code->>LDR: readCorrectedLight()
        alt Temp > 30°C
            Code->>Buzzer: triggerBuzzer()
        else Light < Threshold
            Code->>LED: digitalWrite(HIGH)
        end
        Code->>OLED: updateDisplay()
        Code->>Serial: printReadings()
    end
```

To switch from auto to manual the logic is handled as follows

```mermaid
sequenceDiagram
    participant User
    participant Button
    participant Microcontroller
    participant OLED
    participant Serial

    User->>Button: Presses button
    Button->>Microcontroller: DigitalRead(LOW)
    Microcontroller->>Microcontroller: delay(50ms) // Debounce
    loop Debounce Check
        Button->>Microcontroller: DigitalRead(LOW)
    end
    Microcontroller->>Microcontroller: autoMode = !autoMode
    alt Switching to Auto Mode
        Microcontroller->>OLED: clearDisplay()
        Microcontroller->>OLED: print("Mode: Auto")
        Microcontroller->>Serial: println("Mode: Auto")
    else Switching to Manual Mode
        Microcontroller->>OLED: clearDisplay()
        Microcontroller->>OLED: print("Mode: Manual")
        Microcontroller->>Serial: println("Mode: Manual")
    end
    loop Wait for Release
        Button->>Microcontroller: DigitalRead(HIGH)
    end
    Microcontroller->>OLED: display()
```

Upon switching from manual the logic of the program is as follows

```mermaid
flowchart TB
    Start --> SerialAvailable{Serial Available?}
    SerialAvailable -->|Yes| GetCommand[Get Command]
    GetCommand --> Switch{Command Type?}
    
    Switch -->|'l'| ToggleLED[Toggle LED]
    Switch -->|'b'| SoundBuzzer[Sound Buzzer]
    Switch -->|'r'| ReadSensors[Read Sensors]
    Switch -->|'?'| ShowHelp[Show Help]
    
    ToggleLED --> UpdateDisplay
    SoundBuzzer --> UpdateDisplay
    ReadSensors --> UpdateDisplay
    ShowHelp --> End
    
    UpdateDisplay --> End
```

There was need to define additional function such as `triggerBuzzer()` which ensured the buzzer behaved as intended since `digitalWrite()` did not work as well as `readCorrectedLight()` to map the values of the photoresistor 'correctly' since they were inverted.

Instead of using a capacitor to handle debouncing of the button, it was accounted for in the logic of the program, this requires the user to hold the button for at least 50 milliseconds for the mode to switch.

The readings were stored in a circular buffer, and printed during each refresh.

# Reason For Using C++
I picked C++ as I was using a [wokwi.com](https://wokwi.com/) and it is the default language.
Additionally the project was not extremely complicated so I didn't run into issues like race conditions for example.

