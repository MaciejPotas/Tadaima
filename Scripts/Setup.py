import subprocess
import sys
import os

def install_virtualenv():
    """Ensure that pip is up-to-date and virtualenv is installed."""
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', '--upgrade', 'pip'])
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'virtualenv'])

def create_virtualenv(env_name="venv"):
    """Create a virtual environment."""
    subprocess.check_call([sys.executable, '-m', 'virtualenv', env_name])

def install_requirements(env_name="venv"):
    """Install the required dependencies in the virtual environment."""
    if os.name == 'nt':
        activate_script = os.path.join(env_name, 'Scripts', 'activate.bat')
        command = f'cmd /c "{activate_script} && pip install -r requirements.txt"'
    else:
        activate_script = os.path.join(env_name, 'bin', 'activate')
        command = f'bash -c "source {activate_script} && pip install -r requirements.txt"'
    
    subprocess.check_call(command, shell=True)

if __name__ == "__main__":
    install_virtualenv()
    create_virtualenv()
    install_requirements()
