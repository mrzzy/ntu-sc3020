from interface import GUI
from dotenv import load_dotenv


def main():
    load_dotenv()
    app = GUI()
    app.run()


if __name__ == "__main__":
    main()
