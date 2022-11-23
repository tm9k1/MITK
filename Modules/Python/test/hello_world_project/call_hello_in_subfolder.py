from hello_subfolder.hello import HelloToCallInFolder
if __name__ == "__main__":
    hi = HelloToCallInFolder()
    hi.text = "Hello from outside the subfolder!"
    hi.print_text()
