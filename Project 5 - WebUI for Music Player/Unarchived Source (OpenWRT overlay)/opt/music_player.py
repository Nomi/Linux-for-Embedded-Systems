from mpd import MPDClient
import mpd
import gpiod
import time

consumerString="CONSUMER"
mpdIP = "127.0.0.1" #"0.0.0.0" #localhost
mpdPort = 6600

gpiodButtonOffsets = [25,10,17,18]


def eventtype_debounced(line, timeout):
	event = 0
	new = False
	ev_line = line.event_wait(sec=timeout)
	while ev_line:
		new = True
		event = line.event_read()
		ev_line = line.event_wait(0,150000000)
	if new == False :
		return event
	else :
		return event.type

def bulk_eventtype_debounced(lines, timeout):
	event = 0
	result = lines.event_wait(sec=timeout)
	if result != None :
		line = result[0]
		read_val = line.event_read()
		olevent = read_val.type
		event = eventtype_debounced(line, timeout)
		if event == 0 :
			event = olevent
		if event == 2 and str(line) == str(lines.to_list()[0]) :
			event = gpiodButtonOffsets[0]
		elif event == 2 and str(line) == str(lines.to_list()[1]) :
			event = gpiodButtonOffsets[1]
		elif event == 2 and str(line) == str(lines.to_list()[2]) :
			event = gpiodButtonOffsets[2]
		elif event == 2 and str(line) == str(lines.to_list()[3]) :
			event = gpiodButtonOffsets[3]
		# elif event == 2 and str(line) == str(lines.to_list()[4]) :
		# 	event = 18
	return event

client = MPDClient()
client.idletimeout = None
client.connect(mpdIP, mpdPort)
client.play()

def reconnect():
    try:
        client.ping()
    except Exception:
        client.connect(mpdIP, mpdPort)
        client.update()

stopped = False

chip = gpiod.Chip('gpiochip0')
led27 = chip.get_line(27)
led27.request(consumer=consumerString, type=gpiod.LINE_REQ_DIR_OUT)
led27.set_value(0)

buttons = chip.get_lines(gpiodButtonOffsets)
buttons.request(consumer=consumerString, type=gpiod.LINE_REQ_EV_BOTH_EDGES)
while True:
	currentEvent = bulk_eventtype_debounced(buttons, 2)
	if currentEvent == gpiodButtonOffsets[0] :
		print("--previous--")
		reconnect()
		client.previous()
	elif currentEvent == gpiodButtonOffsets[1] :
		if stopped == False :
			led27.set_value(1)
			stopped = True
			reconnect()
			client.pause()
			print("--pause--")
		else :
			led27.set_value(0)
			stopped = False
			reconnect()
			client.play()
	elif currentEvent == gpiodButtonOffsets[2] :
		print("--next--")
		reconnect()
		client.next()
	elif currentEvent == gpiodButtonOffsets[3] :
		print("--volume_down--")
		reconnect()
		volume1 = (int)(client.status()['volume'])
		if volume1 >= 20 :
			reconnect()
			client.setvol(volume1 - 20)
	# elif currentEvent == 18 :
	# 	reconnect()
	# 	volume2 = (int)(client.status()['volume'])
	# 	if volume2 <= 80 :
	# 		reconnect()
	# 		client.setvol(volume2 + 20)
	time.sleep(0.05) #In case debouncing code is incorrect :O
