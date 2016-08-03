import userinterface
import unittest
import time
import colorama
from functools import wraps

class BlinkinlabsTestRunner():
  """
  A test runner class that logs to a databse and displays results to the screen
  """
  def __init__(self, failfast=False):
    self.interface = userinterface.interface
    #self.l = Logger.logger

    self.failfast = failfast
    pass

  def run(self, test):
    "Run the given test case or test suite."
    result = BlinkinlabsTestResult()

    #registerResult(result)
    result.failfast = self.failfast
    #result.buffer = self.buffer
    startTime = time.time()
    startTestRun = getattr(result, 'startTestRun', None)
    if startTestRun is not None:
        startTestRun()
    try:
        test(result)
    finally:
        stopTestRun = getattr(result, 'stopTestRun', None)
        if stopTestRun is not None:
            stopTestRun()
    stopTime = time.time()
    timeTaken = stopTime - startTime

    result.printErrors()

    run = result.testsRun
    output = "Ran %d test%s in %.3fs\n" % (run, run != 1 and "s" or "", timeTaken)
    if not result.wasSuccessful():
      failed, errored = map(len, (result.failures, result.errors))
      if failed:
        output += "\nFAILED: %d\n" % failed
      if errored:
        output += "\nERRORS: %d\n" % errored
    else:
      output += "\nALL OK!\n"
    self.interface.DisplaySeparator()
    self.interface.DisplayMessage(output)
      
    return result

def failfast(method):
    @wraps(method)
    def inner(self, *args, **kw):
        if getattr(self, 'failfast', False):
            self.stop()
        return method(self, *args, **kw)
    return inner


class BlinkinlabsTestResult(unittest.TestResult):
  """A test result class that can log formatted text results to a stream.

  Used by BlinkyTapeTestRunner.
  """
  def __init__(self):
    unittest.TestResult.__init__(self)
    self.interface = userinterface.interface
    #self.l = Logger.logger

    self.failfast = False

  def getDescription(self, test):
    return test.shortDescription() or str(test).split(" (")[0]

  def startTest(self, test):
    self.interface.DisplaySeparator()
    self.interface.DisplayMessage(self.getDescription(test), fgcolor=colorama.Fore.BLUE)

    unittest.TestResult.startTest(self, test)
    #self.l.TestStart(test)

#  def stopTest(self, test):
#     self.shouldStop = test.stopTests

  def addSuccess(self, test):
    unittest.TestResult.addSuccess(self, test)
    self.interface.DisplayPass()
    #self.l.TestPass(test)

  @failfast
  def addError(self, test, err):
    unittest.TestResult.addError(self, test, err)
    self.interface.DisplayError()
    #self.l.TestError(test, err)

  @failfast
  def addFailure(self, test, err):
    unittest.TestResult.addFailure(self, test, err)
    self.interface.DisplayFail()
    #self.l.TestFail(test)

  def printErrors(self):
    self.printErrorList('ERROR', self.errors)
    self.printErrorList('FAIL', self.failures)

  def printErrorList(self, flavour, errors):
    for test, err in errors:
      self.interface.DisplayMessage("%s: %s" % (flavour,self.getDescription(test)))
      self.interface.DisplayMessage("%s" % err)


