from dotenv import load_dotenv

from interface import GUI


def main():
    load_dotenv()
    app = GUI()
    app.run()


if __name__ == "__main__":
    main()
