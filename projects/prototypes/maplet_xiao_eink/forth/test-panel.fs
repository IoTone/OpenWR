\ test-panel.fs — bench check of the e-paper on the S3 Sense
status
epd-clear
4 4 204 96 epd-frame
2 epd-scale
s" MAPLET S3" 24 20 epd-str
1 epd-scale
s" XIAO ESP32S3 SENSE 212x104" 24 44 epd-str
20 70 180 70 epd-line
epd-show
epd-slot@ .
