from flask import Flask,render_template

app = Flask(__name__)

@app.route('/')
def LogIn():
    return render_template('TrangChu.html')

if __name__ == '__main__':
    app.run(debug=True)