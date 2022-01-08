import tensorflow as tf
import numpy as np
import pandas as pd
import math
import pickle

from tensorflow.keras import layers
from matplotlib import pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import train_test_split

import random

selected = "data.csv"

df = pd.read_csv(selected, sep=',', decimal='.')
df.drop("dt", axis=1, inplace=True)
label = "pid"

features = [name for name in df.keys() if name != label]
X = df[features].values
y = np.reshape(df[label].values, (-1, 1))

s_x = MinMaxScaler()
X = s_x.fit_transform(X)

X = np.reshape(X, (X.shape[0], X.shape[1], 1))

# shuffling data
N_samples = X.shape[0]
rand_idxs = list(range(N_samples))
random.shuffle(rand_idxs)
X = np.array([X[i] for i in rand_idxs])
y = np.array([y[i] for i in rand_idxs])

test_split_ratio = 0.2
split_idx = int(N_samples * (1-test_split_ratio))
train_data = {'x': X[:split_idx], 'y': y[:split_idx]}
test_data = {'x': X[split_idx:], 'y': y[split_idx:]}

#from sklearn.feature_selection import *
#
#best = SelectKBest(score_func=f_regression, k='all')
#s = 0
#e = 50
#names = ["imu_"+str(i) for i in range(s, e+1)]
#fit = best.fit(np.reshape(X, (-1, 51)), y)
#with plt.style.context("seaborn"):
#  fig = plt.figure(figsize=(12, 12), dpi=80)
#  ax = fig.add_subplot(111)
#  ax.bar(x = names, height=fit.scores_)
#  plt.xticks(rotation=90)
#  plt.show()

def build_model(input, tflite=False):
  
  model = tf.keras.models.Sequential()
  if tflite:
    model.add(tf.keras.layers.Input(input.shape[1:], batch_size=1))
  else:
    model.add(tf.keras.layers.Input(input.shape[1:]))
  model.add(tf.keras.layers.LSTM(128, return_sequences=True))
  model.add(tf.keras.layers.Dropout(rate=0.1))
  model.add(tf.keras.layers.LSTM(50))
  model.add(tf.keras.layers.Dropout(rate=0.1))

  model.add(tf.keras.layers.Dense(units=1))

  opt = tf.keras.optimizers.Adam(learning_rate = 0.0003);
  model.compile(optimizer=opt,
                loss='mse',
                metrics=[tf.keras.metrics.MeanSquaredError()])
  return model

model = build_model(X)

es = tf.keras.callbacks.EarlyStopping(monitor='val_loss',
                   mode='min',
                   verbose=1,
                   patience=100
                  )

try:
    history = model.fit(
        train_data['x'], train_data['y'],
        batch_size=16,
        epochs=1000,
        validation_split=0.2,
        callbacks=[]) 

    with open('history.dat', 'wb') as f:
      pickle.dump(history, f)
except:
    model.save_weights("lstm.h5")
    exit

model.save_weights("lstm.h5")

litemodel = build_model(X, tflite=True)
litemodel.set_weights(model.get_weights())

converter = tf.lite.TFLiteConverter.from_keras_model(litemodel) # path to the SavedModel directory
tflite_model = converter.convert()

# Save the model.
with open('lstm.tflite', 'wb') as f:
  f.write(tflite_model)

