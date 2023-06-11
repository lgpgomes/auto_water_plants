let temperaturaDiv = document.getElementById('temperature')
let iconImg = document.getElementById('iconWeather')
let descriptionDiv = document.getElementById('description')
let saudationDiv = document.querySelector('.welcome-left > h2')
let timeDiv = document.getElementById("time");

var url = 'https://api.openweathermap.org/data/2.5/weather?q=Wenceslau%20Guimar%C3%A3es&appid=ed56f6df337df85be24e6c862dadad5e&units=metric'
var countCallsAPI = 0

let intervalCalls = setInterval(function() {
    countCallsAPI = 0;
    console.log('reset')
}, 60000)

function getWeather() {
    if(countCallsAPI < 60) {
        countCallsAPI++
        fetch(url)
        .then(response => response.json())
        .then(data => {
            const icon = `https://openweathermap.org/img/wn/${data.weather[0]["icon"]}@2x.png`
            temperaturaDiv.innerHTML = Math.round(data.main.temp)
            iconImg.src = icon
            descriptionDiv.innerHTML = data.weather[0].description
        })
        .catch(() => {
        })
    }
}

function timeNow() {
    let now = new Date()
    if(now.getHours() > 5  && now.getHours() <= 12) saudationDiv.innerHTML = 'Bom dia!'
    else if (now.getHours() > 12 && now.getHours() <= 18) saudationDiv.innerHTML = 'Boa tarde!'
    else saudationDiv.innerHTML = 'Boa noite!'

    timeDiv.innerHTML = now.toLocaleString('pt-BR', {timeZone: 'America/Sao_Paulo'})
}

setInterval(timeNow, 1000);

getWeather()
