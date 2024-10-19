# Revolute
- nRF52833 Based Scroll wheel dial
<p align="center"><img src="./Misc/Pictures/Revolute-On_Keyboard.png" width="70%"></p>

## Introducing Revolute

### The Revolute is a scroll wheel/dial that aims to revolutionize the way we interact with complex software applications.
- For engineers and artists, no matter professionals or hobbyists, the use of tedious complex computer software programs are inevitable when it comes to designing and creating. With all the numbers and sliders to manage, it becomes a mentally stressing job to click back and forth between all the parameters. With the Revolute, you can replace many of these actions with the simple rotation of the wheel. Actions such as zooming in and out, changing brush sizes accurately, scrolling and adjusting timelines can all be configured to work with the Revolute when it comes to photo/video editing, graphics designing, 3D modelling/animating, CAD and many of these computer programs.

## How the Revolute works

### Rotational inputs
- The Revolute was originally designed to use mechanical encoders just like the scroll wheels on most mouses nowadays, which often face many durability issues due to the nature of the mechanism. However, with the magnetic encoder that we have managed to utilize to our full advantage, it solves issues of conventional encoders and gives us more space with innovation and creativity with this product.
- The AS5600 magnetic encoder by ams OSRAM records the angle of the magnet's magnetic poles when place opposing to it, which is then recorded and sent to the SoC through I2C. When the wheel is turned and the angle changes, the SoC will notice and proceed its functionalities based on its configured settings.

### BLE connection
- Using BLE wireless connection we can have the Revolute connected to devices such as your computer, phone, or tablet using bluetooth. 
- By using the nRF52833 SoC and Zephyr RTOS, the Revolute is designed to maximize its power efficiency for bluetooth functionalities.

### Central button
- By pressing down on the Revolute, it powers on the device.
- After powering on the device, the button can also be configured to other actions based on your configurations
- Power it off by pressing and holding down on the button.

### Configuration
- As mentioned before, the Revolute's functionalities can be configured through the Revolute Configuration app which you can download from the website <a href = "https://tongtonginc.com/revolute">.
- Once you have made your changes to our configurations, they are then sent to the Revolute over bluetooth and stored on the SoC. This allows it so that there won't be any kind of cloud account required to store these configurations or any data stored locally while being able to use the same configurations across devices.
- For more details on how the configuration app works, visit <a href = "">

### Charging and Battery Replacement
- For this device to be wireless, the Revolute is comprised of a 70Mah li-Ion rechargable coin battery (LIR2032H)
- The Revolute uses the BQ24075-RGT charger ic and BQ27441-G1 fuel gauge, both by Texas Instruments, for charging, monitoring, and managing the battery that powers the system.
- To charge the Revolute, simply take your Revopod (should come with the Revolute) and mount it in the pod and have the two pogopins aligned with the receptors on the wheel.
- To replace the battery, simply flip the Revolute over, press it down until you feel the central button being completely pressed down, and twist it clockwise and you should be able to have the entire Revolute disassembled. From there, simply place a new battery in and reverse the steps of how you disassembled it after your align the cap with the body chassis.

### *Gyro mouse
- With the LSM6DS3 gyroscope accelerometer incorporated in on the Revolute, you can configure it to control the movements of your cursor by moving the Revolute around.
- This feature is still in development and will be finalized shortly in our next firmware updates

## Get your own Revolute

- Go and check us out on our crowdfund campaigns
- For more information head to <a href = "https://tongtonginc.com/revolute"> 
- Feel free to DM us if you have any questions or requests

## Build your own Revolute

- If you want to build or customize your own Revolute, you can find instructions and documentations at <a href = "https://documentation.tongtonginc.com/">.
- Feel free to share your builds or newly developed features with us.



