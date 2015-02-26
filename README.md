                      +---------------------+
                      | Awsedit README file |
                      +---------------------+ 

Inroduction
-----------
Awsedit version 0.1 is the latest release (2011).

Awsedit is a GUI Utility that lets you view and extract data from an
AWS Virtual Tape File.

Purpose of Awsedit
------------------
First Awsedit is intented for people who want to recover data from tapes or
cartridges that come from Mainframes.
The data on tape media may be either in EBCDIC or ASCII.

While extracting data, some operations on fields of data files are allowed only
when in EBCDIC. (For explanation, refer to awsedit.pdf in doc directory)

Creating AWS Virtual Tape Files
-------------------------------
To read data from tapes or cartridges you need, of course, the corresponding
reader device.

You have to connect the reader device to a computer running Linux, with the hope
that this device will be detected.Then install Hercules package from 
<http://www.hercules-390.org/>.

Finally, use Tapecopy (from hercules package) to read your tapes and create the 
AWS Virtual Tape Files.

Installing Awsedit
------------------
Follow instruction in the INSTALL file.

In case of issues
-----------------
Refer to the FAQ file.


The Author
----------
Mustapha Oldache <mustapha.oldache@gmail.com>

