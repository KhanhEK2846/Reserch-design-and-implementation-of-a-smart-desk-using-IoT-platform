from flask import Flask,render_template,request


app = Flask(__name__)

@app.route('/',methods=['GET', 'POST'])
def LogIn():
    if request.method == "GET":
        return render_template('TrangChu.html')
    if request.method == "POST":
        return

@app.errorhandler(404)
def page_not_found(error):
    return render_template("404.html"), 404

if __name__ == '__main__':
    app.run(debug=True)