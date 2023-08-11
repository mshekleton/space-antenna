import pyttsx3
# Remember to sudo apt install espeak

from tones import SINE_WAVE, SAWTOOTH_WAVE
from tones.mixer import Mixer
from playsound import playsound

engine = pyttsx3.init()
rate = engine.getProperty('rate')
print(rate)
engine.setProperty('rate', 125)
voices = engine.getProperty('voices')
engine.setProperty('voice', 'english-us')

mixer = Mixer(44100, 0.5)

mixer.create_track(0, SAWTOOTH_WAVE, vibrato_frequency=7.0, vibrato_variance=30.0, attack=0.01, decay = 0.1)
mixer.create_track(1, SINE_WAVE, attack=0.01, decay=0.1)
mixer.add_note(0, note='c#', octave=5, duration=1.0, endnote='f#')
mixer.add_note(1, note='f#', octave=5, duration=1.0, endnote='g#')
#mixer.write_wav('tones.wav')
samples = mixer.mix()
playsound('tones.wav')
# for voice in voices:
#     engine.setProperty('voice', voice.id)
#     engine.say('fuck you')
#     print(str(voice.id))
#     engine.runAndWait()

# while 1:
#     say = input("What do you want to say? ")
#     engine.say(say)
#     engine.runAndWait()