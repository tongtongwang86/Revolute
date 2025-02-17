View this project on [CADLAB.io](https://cadlab.io/project/28847). 

# Revolute
- nRF52833 Based Scroll wheel dial
<p align="center"><img src="./Misc/Pictures V3/poster/herofull.png" width="70%"> </p>

<p align="center">
    <img src="./Misc/Pictures/Revolute-On_Keyboard.png" width="45%">
    <img src="./Misc/Pictures V3/poster/design.png" width="45%">
</p>



## Introducing Revolute
 <a href="https://tongtonginc.com/revolute">Visit the product page website (Faster loading time, Interactive 3D Models, Demo Videos)</a>

### The Revolute is a scroll wheel/dial that aims to revolutionize the way we interact with complex software applications.

- Revolute currently simulate keyboard inputs, consumer controlls, mouse inputs.

- the actions and sensitivity of the wheel can be configured through the revolute configurator app that will be available for iOS, Macos and Windows devices that have bluetooth support.

- Configuratios done on revolute is persistently stored on device, which means that revolute can work on any device with Bluetooth and HID support.

- Revolute has a number of use cases and is not limited to: scrubbing through a timeline in a video editing app, changing the brush size in 3D/2D painting, smooth scrolling, volume controll, brightness control, media playback control, quickly pressing keys in short successions within games. if you get two you can bind each revolute axis to x and y of the cursor, making a etch a sketch

- Gamepad emulation is currently in development, which will allow you to use revolute within a wide range of mobile and console games.

## How the Revolute works

### Rotational inputs
<p align="center"><img src="./Misc/Pictures V3/poster/magneticenc.png" width="70%"> </p>
- Revolute has gone through a number of iterations and currently uses the AS5600 magnetic encoder. It recordes the angle of a diametric magnet embedded within the base of the revolute, which the SoC will process and send bluetooth reports to the host device.

### BLE connection
<p align="center"><img src="./Misc/Pictures V3/poster/connectivity.png" width="70%"> </p>

- Using BLE wireless connection we can have the Revolute connected to devices such as your computer, phone, or tablet using bluetooth. 
- By using the nRF52833 SoC and Zephyr RTOS, the Revolute is designed to maximize its power efficiency for bluetooth functionalities.

### Central button
- By pressing down on the Revolute, it powers on the device.
- After powering on the device, the button can also be configured to other actions based on your configurations
- Power it off by pressing and holding down on the button.

### Configuration
<p align="center"><img src="./Misc/Pictures V3/configurator/configurator.png" width="70%"> </p>


- As mentioned before, the Revolute's functionalities can be configured through the Revolute Configuration app that is currently in development
- Once you have made your changes to our configurations, they are then sent to the Revolute over bluetooth and stored locally on the SoC. allowing you to use the same configurations across different devices.

### Accessories 
- Revolute has a range of existing accessories that allow you to use it within different use cases

- Key attachment 
<p align="center"><img src="./Misc/Pictures V3/accessories/key.webp" width="70%"> </p>

- Adhesive/magnetic attachment
<p align="center"><img src="./Misc/Pictures V3/accessories/adhesive.webp" width="70%"> </p>

- Phone attachment 
<p align="center"><img src="./Misc/Pictures V3/accessories/phone.webp" width="70%"> </p>

- Revo-Pod charges the revolute and is a place to store your attachments.
<p align="center"><img src="./Misc/Pictures V3/accessories/pod.webp" width="70%"> </p>

- The attachment can be hot swapped with a common bearing interface.
<p align="center"><img src="./Misc/Pictures V3/poster/commonattach.png" width="70%"> </p>


### Charging and Battery Replacement
- For this device to be wireless, the Revolute is comprised of a 70Mah li-Ion rechargable coin cell battery (LIR2032H)
- The Revolute uses the BQ24075-RGT charger ic and BQ27441-G1 fuel gauge, both by Texas Instruments, for charging, monitoring, and managing the battery that powers the system.
- To charge the Revolute, simply take your Revopod and mount it in the pod and have the two pogopins aligned with the receptors on the wheel.
- To replace the battery, simply flip the Revolute over, press it down until you feel the central button being completely pressed down, and twist it clockwise and you should be able to have the entire Revolute disassembled. From there, simply place a new battery in and reverse the steps of how you disassembled it after your align the cap with the body chassis.

### *Gyro mouse
- With the LSM6DS3 gyroscope accelerometer incorporated in on the Revolute, you can configure it to control the movements of your cursor by moving the Revolute around.
- This feature is still in development and will be finalized shortly in our next firmware updates

## How this repo works
- PCB hardware files are in the folder /Hardware
- 3D printed part files are stored in the folder /3D Prints
- Latest firmware is hosted on the repo <a href="https://github.com/tongtongwang86/revolute-firmware-ci">"revolute-firmware-ci"</a>
- There is also a wired version of the revolute in this repo

## Get your own Revolute
- Revolute is still in development and will be ready soon

 - <a href="https://tongtonginc.com/revolute">For more information head to the product page</a>

- Feel free to DM us if you have any questions or requests




