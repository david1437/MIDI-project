#!/usr/bin/env python
import pygame, sys, os
import pygame.midi
import inputbox
from pygame.locals import *

pygame.init()

pygame.font.init()
pygame.fastevent.init()
event_get = pygame.fastevent.get
event_post = pygame.fastevent.post

pygame.midi.init()

pygame.display.set_caption("Midi Data Analyzer")
screen = pygame.display.set_mode((400, 300), RESIZABLE, 32)

myfont = pygame.font.Font('freesansbold.ttf', 15)
surfacefont = myfont.render('Port Opened Sucessfully!',True,(255,255,255), (0,0,0))
surfacer = surfacefont.get_rect()
surfacer.center = (200,150)

user_input = str(inputbox.ask(screen, 'Enter Port Number'))

if(user_input != ""):
        try:
                i = pygame.midi.Input( int(user_input) )
        except Exception:
                screen.fill((0,0,0))        
                surfacefont = myfont.render('Invalid Port Number!',True,(255,255,255), (0,0,0))
                screen.blit(surfacefont, surfacer)
                pygame.display.update()
                pygame.time.wait(5000)
                exit()                
else:
        try:
                i = pygame.midi.Input( pygame.midi.get_default_input_id() )
        except Exception:
                screen.fill((0,0,0))        
                surfacefont = myfont.render('No port detected!',True,(255,255,255), (0,0,0))
                screen.blit(surfacefont, surfacer)
                pygame.display.update()
                pygame.time.wait(5000)
                exit()

screen.fill((0,0,0))        
f = open('output.txt', 'w')
f.write('{}    {}    {}    {}    {}    {}\n'.format("Timestamp", "Event Code", "Note Name", "Note Octave", "Velocity", "Duration"))

note_list = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
midi_data_list = []
going = True

time = 0

while going:
        screen.blit(surfacefont, surfacer)
        events = event_get()
        for e in events:
                if e.type in [QUIT]:
                        going = False
                if e.type in [KEYDOWN]:
                        going = False

        if i.poll():
                midi_events = i.read(100)
                for index in range(len(midi_events)):
                        midi_data_list.append(midi_events[index])
                midi_evs = pygame.midi.midis2events(midi_events, i.device_id)

                for m_e in midi_evs:
                        event_post( m_e )
        pygame.display.update()
        
for index, midi_data in enumerate(midi_data_list):
        timestamp = str(midi_data[1])+"ms"
        if midi_data[0][0] < 144:
                note_press = "Off"
                event = True
        else:
                note_press = "On"
                event = False
        if midi_data[0][1] <= 11:
                note = note_list[midi_data[0][1]]
                octave = "0"
        else:
                note = note_list[midi_data[0][1] % 12]
                octave = str(midi_data[0][1]/12)
        if event is True:
                duration = "-"
                velocity = str(midi_data[0][2])
                search_list = midi_data_list[0:index]
                for search_data in reversed(search_list):
                        if search_data[1] < midi_data[1] and search_data[0][0] >= 144 and search_data[0][1] == midi_data[0][1]:
                                duration = str(midi_data[1] - search_data[1])
                                break
        else:
                velocity = str(midi_data[0][2])
                duration = "-"        
        output = '{:10}   {:10}    {:10}   {:10}     {:10}    {:10}\n'.format(timestamp, note_press, note, octave, velocity, duration)
        f.write(output)

f.close()
i.close()
del i
pygame.midi.quit()
pygame.quit()
exit()
