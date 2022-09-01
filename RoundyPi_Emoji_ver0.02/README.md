
Program to display Emoji.
The emoji files are PNG and I got them from the site: "https://sensa.co/emoji/".
The list of names is in an "emoji_list.h" header file. The files are read from SDCard: they are 1.3MB in total
and they don't all flash. For now I want to see them all. Then it will choose which ones to use.
 
This version uses the two MCUs to run the two main JOBs: the MCU0 performs reading and decoding of PNG files
while the MCU1 is displaying it.
