from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.support.ui import Select
import sys
import time

if len(sys.argv) < 2:
    print('url not found')
    sys.exit()

url = sys.argv[1]

options = Options()
# options.add_argument('--no-sandbox')
# options.add_argument('--disable-dev-shm-usage')
options.add_argument('--headless')
options.add_argument("--disable-gpu")
driver = webdriver.Chrome(options=options)
driver.get(url)

username = 'admin'
password = 'password'

# Find the username and password input fields on the login page using various locators
username_field = driver.find_element('name', 'username')
password_field = driver.find_element('name', 'password')

# Enter the username and password into their respective fields
username_field.send_keys(username)
password_field.send_keys(password)

# Find and click the login button
login_button = driver.find_element('name', 'Login')
login_button.click()

# After logging in, navigate to the desired page
driver.get(url + '/security.php')

select_element = driver.find_element('name', 'security')

# Use the Select class to interact with the <select> element
select = Select(select_element)

# Select the option with value "low"
select.select_by_value('low')

submit_button = driver.find_element('name', 'seclev_submit')
submit_button.click()

timeToSleep = 1

images = []
with open('image.txt', 'r') as f:
    images = [line[:-1] for line in f.readlines()]

print('start fake image xss attack')

import random

i = random.randint(0, len(images) -1)
image_url = images[i]

print(f'{i}/{len(images)}')
time.sleep(timeToSleep)
driver.get(url+"/vulnerabilities/xss_r/")
username = driver.find_element('name', "name")
fakeHackImage=f'<img src="{image_url}" height="200" width="200">'
time.sleep(timeToSleep)
username.send_keys(fakeHackImage)
time.sleep(timeToSleep)
submit_button = driver.find_element('xpath', "//input[@type='submit']")
submit_button.click()
#Just that the image the visible for 10 seconds, it takes time to load
time.sleep(1)
driver.get_screenshot_as_file(f"figure/{i}.png")

    

