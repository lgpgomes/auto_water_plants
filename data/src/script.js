let clientJson = []
let form = document.querySelector('form')
let buttonWater = document.querySelector('#toggle-water')
let sidebar = document.querySelector(".sidebar")
let sidebarBtn = document.querySelector(".sidebar-btn")

socket = new WebSocket("ws:/" + "/" + location.host + ":81")
socket.onopen = function(e) {  console.log("[socket] socket.onopen ")}
socket.onerror = function(e) {  console.log("[socket] socket.onerror ")}
socket.onmessage = function(e) {  
    console.log("[socket] " + e.data)
    let dataFromServer = JSON.parse(e.data)
    updateAllElements(dataFromServer)   
    return true;
}

var makeRequest = function (url, method) {
    var request = new XMLHttpRequest()
    return new Promise(function (resolve, reject) {
        request.onreadystatechange = function () {
            if (request.readyState !== 4) return;
            if (request.status >= 200 && request.status < 300) {
                resolve(request)
            } else {
                reject({
                    status: request.status,
                    statusText: request.statusText
                })
            }
        }
        request.open(method || 'GET', url, true);
        request.send();
    })
}

let getData = makeRequest('updateData', 'GET')

getData.then(function (data) {
    let dataFromServer = JSON.parse(data.responseText)
    updateAllElements(dataFromServer)
})

function updateAllElements(dataFromServer) {
    dataFromServer.states.isWatering == 1 ? buttonWater.checked = 1 : buttonWater.checked = 0

    if (JSON.stringify(clientJson.data) !== JSON.stringify(dataFromServer.data)) {
        document.getElementById("inputIntervalWater").value = dataFromServer.variables.intervalWater
        document.getElementById("inputFlowRate").value = dataFromServer.variables.flowRate
        document.getElementById("waterCount").innerHTML = dataFromServer.data.waterCount.total
        document.getElementById("timeCount").innerHTML =  millisToMinutesAndSeconds(dataFromServer.data.timeCount)
        document.getElementById("usedWater").innerHTML = (dataFromServer.data.timeCount * dataFromServer.variables.flowRate/60000).toFixed(1) + 'L'
    } 
    if (JSON.stringify(clientJson.hours) !== JSON.stringify(dataFromServer.hours)) {
        buidListOfHours(dataFromServer)
    }
    clientJson = dataFromServer
}


function millisToMinutesAndSeconds(millis) {
    var minutes = Math.floor(millis / 60000);
    var seconds = ((millis % 60000) / 1000).toFixed(0);
    return minutes + ":" + (seconds < 10 ? '0' : '') + seconds;
}

function buidListOfHours(data) {
    data.hours.length >= 7 ? buttonSubmitSchedule.classList.add('disabled') : buttonSubmitSchedule.classList.remove('disabled')

    let listItem = ""
    let listHours = document.querySelector('.list-hours')
    let daysWeekPortuguese = ['Domingo', 'Segunda', 'Terça', 'Quarta', 'Quinta', 'Sexta', 'Sábado']
    let daysWeek = ['dowSunday', 'dowMonday', 'dowTuesday', 'dowWednesday', 'dowThursday', 'dowFriday', 'dowSaturday']

    data.hours.slice().forEach(function (item, index) {
        listItem += `<div class="item-hour">
                        <div class="wrapper-item-hour">
                            <div class="time">
                                <span class="icon-hour">
                                    <svg width="14px" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512"><path d="M256 512C114.6 512 0 397.4 0 256C0 114.6 114.6 0 256 0C397.4 0 512 114.6 512 256C512 397.4 397.4 512 256 512zM232 256C232 264 236 271.5 242.7 275.1L338.7 339.1C349.7 347.3 364.6 344.3 371.1 333.3C379.3 322.3 376.3 307.4 365.3 300L280 243.2V120C280 106.7 269.3 96 255.1 96C242.7 96 231.1 106.7 231.1 120L232 256z"/></svg>
                                </span>
                                <span>
                                    ${daysWeekPortuguese[daysWeek.indexOf(item.dayWeek)]}
                                </span>
                                <span>
                                    ${item.hour}
                                </span>
                            </div>
                        </div>
                        <span class="delete-icon" onclick="deleteHour(${item.id}, this)">
                            <svg width="14px" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 448 512"> <path d="M135.2 17.69C140.6 6.848 151.7 0 163.8 0H284.2C296.3 0 307.4 6.848 312.8 17.69L320 32H416C433.7 32 448 46.33 448 64C448 81.67 433.7 96 416 96H32C14.33 96 0 81.67 0 64C0 46.33 14.33 32 32 32H128L135.2 17.69zM31.1 128H416V448C416 483.3 387.3 512 352 512H95.1C60.65 512 31.1 483.3 31.1 448V128zM111.1 208V432C111.1 440.8 119.2 448 127.1 448C136.8 448 143.1 440.8 143.1 432V208C143.1 199.2 136.8 192 127.1 192C119.2 192 111.1 199.2 111.1 208zM207.1 208V432C207.1 440.8 215.2 448 223.1 448C232.8 448 240 440.8 240 432V208C240 199.2 232.8 192 223.1 192C215.2 192 207.1 199.2 207.1 208zM304 208V432C304 440.8 311.2 448 320 448C328.8 448 336 440.8 336 432V208C336 199.2 328.8 192 320 192C311.2 192 304 199.2 304 208z"/></svg>
                        </span>
                    </div>`
    })
    listItem == "" ? listHours.style.display = 'none' : listHours.style.display = 'flex'
    listHours.innerHTML = listItem
}

function resetJson(){
    let answer = window.confirm(`Deseja resetar todos os dados?`)
    if (answer) makeRequest('resetJson', 'GET')
}

function deleteHour(id, element) {
    let answer = window.confirm(`Deletar esse horário?`)
    if(answer) {
        element.parentElement.classList.add("disabled")
        makeRequest(`updateSchedules?idToRemove=${id}`, 'GET')
    }
}

function allowEditValue(editButton) {
    let input = editButton.parentElement.parentElement.children[1].children[0]         
    input.disabled = 0
    input.focus()
    editButton.classList.add("disabled")
    editButton.parentElement.lastElementChild.classList.remove("disabled")
}

function saveValue(saveButton, variable) {
    let input = saveButton.parentElement.parentElement.children[1].children[0]
    let value = input.value
    input.disabled = 1
    saveButton.classList.add("disabled")
    saveButton.parentElement.firstElementChild.classList.remove("disabled")
    makeRequest(`updateVariables?${variable}=${value}`, 'GET')
}

form.onsubmit = function(e) {
    e.preventDefault()

    if(clientJson.hours.length >= 7) return
    buttonSubmitSchedule.classList.toggle('loading')    

    let hourValue = document.getElementById('inputSchedule').value
    let dayWeekValue = document.getElementById('inputDayWeek').value
    let updateSchedules = makeRequest(`updateSchedules?hour=${hourValue}&dayWeek=${dayWeekValue}`, 'GET')

    updateSchedules.then(function(){
        buttonSubmitSchedule.classList.toggle('loading')
    })
}


buttonWater.onclick = function() {
    if (buttonWater.checked) makeRequest('waterPlants?power=true', 'GET')
    else makeRequest('waterPlants?power=false', 'GET')    
}

sidebarBtn.onclick = function() {
    sidebar.classList.toggle("active")
}