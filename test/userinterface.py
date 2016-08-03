import time
import colorama

class UserInterface():
  """
  The user interface is a simple class for displaying messages on the LCD screen,
  and reading input from the front keypad.
  """

  def __init__(self):
    colorama.init()

  def DisplayMessage(self, message, fgcolor=None, bgcolor=None):
    """
    Display a message on the screen
    @param message Message to display, can be multi-line
    """

    if not fgcolor == None:
    	print(fgcolor),
    if not bgcolor == None:
    	print(bgcolor)

    print message

    if (not fgcolor == None) or (not bgcolor == None):
    	print(colorama.Style.RESET_ALL)

  def DisplaySeparator(self):
      print "**********************************************************"

  def DisplayPass(self, message = 'PASS', timeout=0):
    """
    Display a pass message to the user, for a given amout of time.
    @param timeout Time to display the message, in seconds
    """
    self.DisplayMessage(message, fgcolor=colorama.Fore.GREEN )
    time.sleep(timeout)

  def DisplayError(self, message = 'ERROR', timeout=0):
    """
    Display a failure message to the user, for a given amout of time.
    @param timeout Time to display the message, in seconds
    """
    self.DisplayMessage(message, fgcolor=colorama.Fore.RED)

  def DisplayFail(self, message = 'FAIL', timeout=0):
    """
    Display a failure message to the user, for a given amout of time.
    @param timeout Time to display the message, in seconds
    """
    self.DisplayMessage(message, fgcolor=colorama.Fore.RED)
    time.sleep(timeout)


# Declare a single instance of the user interface, that all modules can share
# TODO: This is so that new modules can be loaded dynamically and run, but there
# is probably a more elegent way to do this.
interface = UserInterface()

