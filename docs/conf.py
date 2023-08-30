import datetime
import subprocess

project = "pebble-le"
copyright = str(datetime.date.today().year) + ", Lesosoftware"

html_theme = "sphinx_rtd_theme"

extensions = ['breathe']

breathe_projects = {"pebble-le": "xml"}
breathe_default_project = "pebble-le"

subprocess.call('cd ..; doxygen', shell=True)
subprocess.call('sed -i "s/ PEBBLE_LE_EXPORT//g" xml/*.xml',
                shell=True)
