import webbrowser
import urllib2
import pyautogui
import time
from selenium import webdriver

driver = webdriver.Chrome();
driver.get("http://www.my.usf.edu/")
user = driver.find_element_by_id("username")
user = pyautogui.typewrite("dsherrier\t")
pyautogui.typewrite("Wolfman1439!\t")
pyautogui.press("enter")
mainPicture = driver.find_element_by_id("zz1_TopNavigationMenu")
pyautogui.click(185,250)
pyautogui.click(185,275)
