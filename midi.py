import pygame
import pygame.midi
from pygame.locals import *

pygame.init()

pygame.fastevent.init()
event_get = pygame.fastevent.get
event_post = pygame.fastevent.post

pygame.midi.init()
i = pygame.midi.Input( 2 )

pygame.display.set_caption("midi test")
screen = pygame.display.set_mode((400, 300), RESIZABLE, 32)

print "starting"
f = open('output', 'w')
going = True

time = 0

while going:
        events = event_get()
        for e in events:
                if e.type in [QUIT]:
                        going = False
                if e.type in [KEYDOWN]:
                        going = False

        if i.poll():
                midi_events = i.read(10)
                print "MIDI EVENT " + str(midi_events) + "\n"
                for index in range(len(midi_events)):
                        f.write(str(midi_events[index]) + "\n")
                midi_evs = pygame.midi.midis2events(midi_events, i.device_id)

                for m_e in midi_evs:
                        event_post( m_e )

print "exit button clicked."
f.close()
i.close()
del i
pygame.midi.quit()
pygame.quit()
exit()
