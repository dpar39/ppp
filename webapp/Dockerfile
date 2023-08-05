FROM python:3.10.6-slim-bullseye

# Install python and pip
ADD server/requirements.txt /tmp/requirements.txt

# Install dependencies
RUN pip install --upgrade pip
RUN pip3 install --no-cache-dir -q -r /tmp/requirements.txt

# Add our code
RUN mkdir /app
WORKDIR /app
ADD ./server/* /app/
ADD ./www/ /app/www/

# Run the app.  CMD is required to run on Heroku
ENV PORT 80
EXPOSE 80

# ENTRYPOINT gunicorn --bind 127.0.0.1:$PORT app:app
#ENTRYPOINT ["gunicorn", "wsgi"]
CMD gunicorn --bind :80 --workers=4 --worker-class=gevent --log-level=debug wsgi