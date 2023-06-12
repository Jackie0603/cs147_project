#server.py 
app = Flask(name)

data_dict = {}
var = str(-1)
timee = str(-1)

@app.route("/", methods=['GET'])
def process_data():
  global data_dict
  global var
  global timee

  score = request.args.getlist("score")
  timee = request.args.getlist("time")

  print("received scores: ",score)
  print("received time: ",timee)

  for num in range(len(timee)):
    data_dict[timee[num]] = score[num]
    
  print(data_dict)
  if len(data_dict) > 1:
    return render_template("index4.html", data=data_dict)
  else:
    return "not yet..."

if __name__ == "__main__":
  app.run()