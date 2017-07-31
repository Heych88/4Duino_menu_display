# 4Duino_menu_display
4Duino display controller for an industrial cutting device that uses a motor and encoder to perform repetitive product 
measuring and cutting to length activities.

## Prerequisites

#### Hardware

This project uses the [4duino](http://www.4duino.com/) board as the Arduino controller and display. 

It also requires a Micro SD card and a Micro SD card reader for programing.

#### Software

The project requires the installation of [Arduino IDE](https://www.arduino.cc/en/Main/Software) and the 
[4D Workshop IDE](http://www.4dsystems.com.au/product/4D_Workshop_4_IDE). Download the Free version of 
[4D Workshop IDE](http://www.4dsystems.com.au/product/4D_Workshop_4_IDE) 
which is only available on a Windows platform.

## Installing

Install the [Arduino IDE](https://www.arduino.cc/en/Main/Software) and  
[4D Workshop IDE](http://www.4dsystems.com.au/product/4D_Workshop_4_IDE) as per their instructions.

Clone this repository.
```
git clone https://github.com/Heych88/4Duino_menu_display.git
```

## Run the Code

1. Run the installed 4D Workshop IDE and connect the 4duino via USB. Check the IDE is communicating with the 4duino by opening the **Comms** tab. Select the correct Comms Port for the 4duino.

2. Open the repositories program in the IDE. 

3. Plug the Micro SD card into the computer.

4. Press the **Build/Copy/Load** button on the home tab.

5. Remove the SD card from the computer and insert into the 4duino. 

## Usage

This repository provides a 4duino touch menu for use with electronic motors and encoders. This is a working repository 
and is still under development.

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D

## License

This project is licensed under the GNU General Public License v3.0 (https://opensource.org/licenses/GPL-3.0) - see 
the [LICENSE.md](LICENSE.md) file for details.
