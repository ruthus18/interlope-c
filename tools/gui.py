from PyQt6.QtWidgets import QApplication, QWidget, QPushButton

import sys


app = QApplication(sys.argv)

window = QPushButton("Connect")
window.show()


app.exec()
