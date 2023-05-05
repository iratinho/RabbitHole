from colorama import Fore, Style

class Logger:
    def LogAction(message):
        print(Fore.GREEN + "[Action]: " + Style.RESET_ALL + message)

    def LogError(message):
        print(Fore.RED + "[Error]: " + Style.RESET_ALL + message)

    def LogInfo(message):
        print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + message)

    def LogWarning(message):
        print(Fore.YELLOW + "[Info]: " + Style.RESET_ALL + message)
