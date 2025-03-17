# 🧠 Simethesia Simulator

**Simethesia Simulator** is an interactive, real-time patient simulator designed specifically for anesthesia control experiments. It implements validated pharmacokinetic/pharmacodynamic (PK/PD) models to realistically simulate a patient's response to propofol infusion, providing an ideal platform for testing and validating advanced control algorithms (PID, LQR, MPC, etc.) in hardware-in-the-loop (HIL) scenarios using Arduino.

Perfect for students, researchers, biomedical engineers, and medical enthusiasts interested in simulation-based learning and research.

---

## 🚀 Key Features

- **Real-time PK/PD Simulation:** Accurately simulates Bispectral Index (BIS) responses to anesthetic drugs.
- **Customizable Patients:** Set patient-specific parameters (age, weight, height, sex).
- **Clinical Challenges:** Realistic scenarios like hemorrhage, vasodilation, vasoactive drugs, and neuronal instability.
- **Serial Command Interface:** Easy patient and challenge configuration via Arduino’s Serial Monitor.

---

## 🧩 System Architecture
The simulator is part of a modular architecture designed for real-time hardware-in-the-loop experiments:

## 📂 Folder Structure
Here's how the project files are organized:

## 🖥️ Serial Commands

Use the Arduino IDE Serial Monitor to interact with the simulator:

| Command             | Example                  | Description                                      |
|---------------------|--------------------------|--------------------------------------------------|
| **Start simulation**    | `start`                | Starts real-time simulation                     |
| **Stop simulation**     | `stop`                   | Stops simulation                                 |
| **Configure patient**   | `patient M 75 175 30`     | Sets patient sex, weight (kg), height (cm), age (years) |
| **Generate patient** | `generate_patient`        | Creates random patient parameters                |
| **Clinical challenge**| `challenge 1`             | Activates a clinical scenario (0–5)              |

** 🚨 Clinical challenges available:**

Use these challenges to simulate realistic physiological scenarios and test the robustness of your anesthesia controller:

- `0`: **Normal conditions** — Baseline simulation without any disturbances (default).
- `1`: **Hemorrhage** — Simulates blood loss, reducing central compartment volume and increasing drug elimination.
- `2`: **Vasodilation** — Increases blood vessel volume, changing the drug's distribution and reducing concentration effectiveness.
- `3`: **Vasoconstriction** — Decreases blood vessel volume and reduces drug equilibration speed, increasing drug concentration responsiveness.
- `4`: **Vasoactive Drugs** — Simulates administration of drugs altering cardiovascular response, affecting distribution and maximum drug effect.
- `5`: **Neuronal Instability** — Reduces brain sensitivity, decreasing baseline BIS responsiveness to the anesthetic, representing challenging neurological scenarios.

## 🚀 How to Run: Arduino

Follow these steps to run the simulation using an Arduino-compatible board:

### 🛠️ Requirements:

- Arduino IDE ([download here](https://www.arduino.cc/en/software))
- Arduino-compatible microcontroller (e.g., Arduino Uno, Mega, Nano)
- USB cable for Arduino connection

### ⚙️ Setup Instructions:

1. **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/simethesia-simulator.git
    ```

2. **Open the Arduino IDE.**

3. **Load the Simulator code:**
    - Go to `File → Open`.
    - Navigate to the project folder and select the main Arduino file (`src/main.ino`).

4. **Connect your Arduino board via USB:**
    - Select the correct port: `Tools → Port`.
    - Choose your Arduino board model: `Tools → Board`.

5. **Upload the sketch:**
    - Click the upload button (➡️).

6. **Start Simulation:**
    - Open the Arduino Serial Monitor (`Tools → Serial Monitor`), set baud rate to `115200`.
    - Use serial commands (e.g., `start`) or control the simulation directly from the **Simethesia App**.

Now you're ready! 🎉 You can interact with the simulation via Arduino’s serial commands or conveniently from your mobile app.

## 🔗 How to Run the Bridge (Arduino Patient Simulator ↔ Mobile App)
To enable communication between the **mobile application** and the **Arduino Patient Simulator**, you need to run a Python script that acts as a **serial bridge**.  
This allows Bluetooth commands from the mobile app to reach the simulator — and simulator feedback (e.g., BIS, infusion rate) to return to the app.

### ✅ Requirements

- Python **3.7+**
- Arduino board connected via USB and running the simulator firmware
- Mobile app 

#### 📦 Install Python dependencies

```bash
pip install pyserial
```
### 🚀 Steps to Run

1. **Pair your phone with your PC via Bluetooth.**
2. **Connect your Arduino to the PC via USB.**
3. **Clone or navigate to the folder with the bridge script:**
   ```bash
   git clone https://github.com/your-org/your-repo.git
   cd your-repo
   ```
4. Run the Python bridge script:
```bash
    python bluetooth_serial_bridge.py
```

The script will attempt to auto-detect:

- 🔵 The **Bluetooth serial port** (e.g., `COM7`, `/dev/rfcomm0`)
- 🟢 The **Arduino USB port** (e.g., `COM10`, `/dev/ttyUSB0`)

If the ports are **not detected automatically**, the script will prompt you to **enter them manually**.

> 💡 Alternatively, you can set the ports manually by editing the script:
>
> Open `bluetooth_serial_bridge.py` and replace the auto-detection with hardcoded values, for example:
>
> ```python
>bt_port = find_port("COM_NUMBER") 
>arduino_port = find_port("COM_NUMBER")
> ```
5. Send commands such as:
```bash
start
patient M 75 180 30
challenge 1
```
6. You will see logs printed both:

 * On your phone screen
 * In the Python terminal


## 🧪 Discrete-Time PK/PD Models (Euler method)

The simulator uses Euler discretization to numerically solve continuous-time equations, introducing a small computational delay equal to the sampling interval (Ts):

### 1. Pharmacokinetic Model (3-compartment):

#### 📌 Variables Description
- **`c₁`, `c₂`, `c₃`**: Concentrations in central and peripheral compartments (shallow and deep).
- **`V₁`, `V₂`, `V₃`**: Volumes of distribution for each compartment.
- **`k₁₀`**: Elimination rate constant from the central compartment.
- **`k₁₂`, `k₂₁`, `k₁₃`, `k₃₁`**: Transfer rate constants between compartments.
- **`u`**: Infusion rate (drug input rate).
- **`Ts`: Discrete sampling interval (simulation step size)

#### 📐 Model Equations
The three-compartment pharmacokinetic model equations in continuous-time are:

```text
dc₁/dt = (u / V₁) − (k₁₀ + k₁₂ + k₁₃) × c₁ + (V₂ / V₁) × k₂₁ × c₂ + (V₃ / V₁) × k₃₁ × c₃  
dc₂/dt = (V₁ / V₂) × k₁₂ × c₁ − k₂₁ × c₂  
dc₃/dt = (V₁ / V₃) × k₁₃ × c₁ − k₃₁ × c₃
```
Discretized equation using Euler:

```text
c₁[k+1] = c₁[k] + Ts × [(u[k]/V₁) − (k₁₀ + k₁₂ + k₁₃)c₁[k] + (V₂/V₁)×k₂₁×c₂[k] + (V₃/V₁)×k₃₁×c₃[k]]
c₂[k+1] = c₂[k] + Ts × [(V₁/V₂)×k₁₂×c₁[k] − k₂₁×c₂[k]]
c₃[k+1] = c₃[k] + Ts × [(V₁/V₃)×k₁₃×c₁[k] − k₃₁×c₃[k]]
```

### 2. Effect-site Compartment (Pharmacodynamic Model):

#### 📌 Variables Description
- `Ce`: Effect-site concentration (responsible for clinical effect)
- `ke₀`: Rate constant determining the speed of drug equilibration between plasma and effect-site compartment
- `c₁`: Central compartment (plasma) concentration
- `Ts`: Discrete sampling interval (simulation step size)

#### 📐 Model Equations
The pharmacodynamic (PD) model describes the relationship between plasma drug concentration and clinical effect, represented by the effect-site concentration (`Ce`). The continuous-time equation is:

```text
dCe/dt = ke₀ × (c₁ − Ce)
```
Discretized equation using Euler:

```text
Ce[k+1] = Ce[k] + Ts × ke₀ × (c₁[k] − Ce[k])
```
### 3. BIS Calculation:

#### 📌 Variables Description
- `BIS`: Bispectral Index (0–100)  
- `E₀`: Baseline BIS (~95–100, awake state)  
- `Emax`: Maximum drug effect  
- `Ce₅₀`: Concentration at 50% of maximal effect  
- `γ` (gamma): Steepness of the drug-response curve  
- `Ce`: Effect-site concentration from PD model

#### 📐 Model Equations

The BIS equation in continuous-time are:

```text
BIS(t) = E₀ − (Emax × Ce(t)^γ) / (Ce₅₀^γ + Ce(t)^γ)
```

Discretized equation using Euler:

```text
BIS[k] = E₀ − (Emax × Ce[k]^γ) / (Ce₅₀^γ + Ce[k]^γ)
```


## ⚠️ **Important Note on Simulation Delay**

The discretized equations used in this simulation introduce a computational delay equal to one sampling interval (`Ts`).  
This occurs because variables calculated at step `[k+1]` depend explicitly on values from the previous step `[k]`, placing the results one sampling step into the future.

This one-step delay (`Ts`) accurately represents the inherent computational delay observed in real-world digital control systems. When developing and testing your control algorithms, remember to account for this delay to ensure accurate performance evaluation and realistic simulation outcomes.

## 🎓 Who Is This For?

- **Students:** Test and explore control theory concepts through practical, hands-on simulation.
- **Educators:** Facilitate interactive, realistic classroom demonstrations and experiments.
- **Researchers:** Validate and refine new control algorithms safely before clinical application.
- **Medical enthusiasts:** Safely experiment and gain insights into anesthesia control and patient dynamics.

## 🤝 Contributing

Contributions are welcome! Help us grow by:

- Reporting issues or suggesting new features
- Creating new patient profiles or clinical scenarios
- Improving documentation or enhancing the simulation code

Together, let's make anesthesia simulation accessible, educational, and engaging for everyone.

