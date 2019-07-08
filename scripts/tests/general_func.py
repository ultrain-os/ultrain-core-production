import time
import subprocess

def cmd_exec( command ):
    print( command )
    subprocess.call( command, shell=True )

def sleep( t ):
    print('sleep', t, '...')
    time.sleep(t)
