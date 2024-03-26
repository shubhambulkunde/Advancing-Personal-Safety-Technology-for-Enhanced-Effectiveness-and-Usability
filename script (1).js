import {
  initializeApp
} from 'https://www.gstatic.com/firebasejs/9.7.0/firebase-app.js'

import { getDatabase, ref, onValue, child, push, update, set } from "https://www.gstatic.com/firebasejs/9.7.0/firebase-database.js";
// Add Firebase products that you want to use

import {
  getAuth,
  GoogleAuthProvider,
} from 'https://www.gstatic.com/firebasejs/9.7.0/firebase-auth.js'

const firebaseConfig = {

  apiKey: "AIzaSyCykVRigfQRzBWaIlQwMBZrcSiGWRgsy9E",

  authDomain: "personalsafety3.firebaseapp.com",

  databaseURL: "https://personalsafety3-default-rtdb.firebaseio.com",

  projectId: "personalsafety3",

  storageBucket: "personalsafety3.appspot.com",

  messagingSenderId: "372124255545",

  appId: "1:372124255545:web:c0a16f0ec96ffb370c94e8"

};
const app = initializeApp(firebaseConfig);
const db = getDatabase();
const DataRef = ref(db, "S1/");
const dataMap ={      'IR': '533',
               'Fall': '0',
               'Lat': '19.60106',
               'Lon': '74.19731',
               'SOS': '0'};

var $ = function(id) { return document.getElementById(id); };

const map = L.map('map').setView([dataMap['Lat'], dataMap['Lon']], 12);

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

L.marker([dataMap['Lat'], dataMap['Lon']]).addTo(map)
  .bindPopup('Device Location')
  .openPopup();

function updateGPS(data){

  L.marker([data['Lat'], data['Lon']]).addTo(map)
    .bindPopup('Device Location')
    .openPopup();
}

function updateMap(newData){
  let splitValues=newData.iot.slice(newData.iot.indexOf('s')+1).split('.');
  dataMap['IR']=splitValues[0];
  dataMap['Fall']=splitValues[1];
  dataMap['Lat']=newData.lat;
  dataMap['Lon']=newData.lon;
  dataMap['SOS']=(newData.SOS=="1" || splitValues[2]=="1" )? "1" : "0";
  console.log(dataMap);
  updateGPS(dataMap);
  $("heartRateStatus").innerHTML=(dataMap['IR'] > 15000) ? 'Normal' : 'Abnormal';
  $("fallDetectionStatus").innerHTML=(dataMap['Fall']=="1") ? 'Fall Detected' : 'Normal';
  $("sosStatus").innerHTML=(dataMap['SOS']=="1") ? 'Unsafe' : 'Safe';
}
onValue(DataRef, (snapshot) => {
  console.log(snapshot.val());
  updateMap(snapshot.val());
});

const phoneForm = document.getElementById('phoneForm');

phoneForm.addEventListener('submit', (e) => {
  e.preventDefault();

  // Get the phone number from the form
  const phoneNumber = document.getElementById('phoneNumber').value.trim();

  // Push the phone number to Firebase
  if (phoneNumber) {
    const updates = {};
    updates['/phone/'] = phoneNumber;

    update(ref(db, "S1/"), updates).then(() => {
        // Clear the input field after successfully adding the number
        document.getElementById('phoneNumber').value = '';
        //alert('Phone number added successfully!');
      })
      .catch((error) => {
        console.error('Error adding phone number: ', error);
      });

  }
});

document.getElementById('sosButton').addEventListener('click', sendSOS);
function sendSOS() {
  const updates = {};
  dataMap["SOS"]=(dataMap["SOS"]=="0")?"1":"0";  
  updates['/SOS/'] = dataMap["SOS"];
  update(ref(db, "S1/"), updates)
}
/*
const statusIndicator = $('statusIndicator');
const statusText = $('statusText');
const isDeviceOnline = true; // Change this based on actual device status

if (isDeviceOnline) {
  statusIndicator.classList.add('bg-green-500');
  statusIndicator.classList.remove('bg-red-500');
  statusText.textContent = 'Device Online';
} else {
  statusIndicator.classList.add('bg-red-500');
  statusIndicator.classList.remove('bg-green-500');
  statusText.textContent = 'Device Offline';
}*/