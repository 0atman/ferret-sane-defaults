build:
	java -jar `which ferret` --release -i f.clj
	g++ -static -std=c++11 -pthread f.cpp -o app
	./app

clean:
	rm *.cpp

watch:
	java -jar `which ferret` -wi f.clj

ast:
	java -jar `which ferret` --ast -i f.clj
