# XIAO_IBUS_Neopixel
Seeeduino XIAO Neopixel control PoC


Throttle (Ch3) - Brightness
Pitch (Ch1) - Speed

PotA - Hue
PotB - Saturation

SwA - Color/Cylon Direction (Left / Right)
SwB - Middle-Out (Enable / Disable)
SwC - Animation Type (Color Wipe / Cylon / Solid)
SwD - Linear Blend (Enable / Disable)



Wheel Position Input
absolute position percentage of 255



output 255,255,255,255




SwD (high): Adjust Selected Color Profile
 - AdjustColorProfile()
  - SwC (1) w/ SwD (1000) = Hue
   - VrA = Adjust Hue values
  - SwC (2) w/ SwD (1500) = Saturation
   - VrA = Adjust Saturation values
  - SwC (3) w/ SwD (2000) = White
   - VrA = Adjust white values up and decrease RGB channels equivalently
  - SwB (low) = Coarse value tune
  - SwB (high) = Fine value tune
 - VrB = Brightness
  - Right Stick (left): Cycle Color Profile Selection (blink n+1 times for color profile number)
   - SelectColorProfile()
  - Right Stick (right): Cycle Color Profile Selection (blink n+1 times for color profile number)
   - SelectColorProfile()
  - Right Stick (up): Show current color profile saved values
   - GetColorProfile()
  - Right Stick (down): Save adjusted color profile values
   - SaveColorProfile()

SwD (low): 
- SwA (low): Standby
- SwA (high): Begin selected state
- SwB (low): Lock values
- SwB (high): Unlock speed/brightness controls
 - VrA = Animation Speed
 - VrB = Brightness
 - SwC (1000): Select animation
  - SelectAnimation()
 - SwC (1500): Change animation direction
  - SetAnimationDirection()
   - Right Stick (right): Animation forward
   - Right Stick (left): Animation reverse
   - Left Stick (right): Normal end-to-end
   - Left Stick (left): Enable middle-out
   - Right Stick (down): Save direction

SelectColorProfile()
- SwD (low): Show Selected Color Profile (where applicable)
 - Right Stick (right): Cycle Up Color Profile Selected (blink n times for color profile number)
  - Increase currColorProfile index number (roll over to 0 if index count exceeded) 
  - ColorProfileBlink() 
 - Right Stick (left): Cycle Down Color Profile Selected (blink n times for color profile number) 
  - Decrease currColorProfile index number (roll over to index count if 0)
  - ColorProfileBlink() 

- ColorProfileBlink()
 - 

