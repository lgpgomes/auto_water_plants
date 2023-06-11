var scheduleTBtn = document.querySelector('#schedule')
var modalSchedule = document.querySelector('#modalSchedule')
var modalScheduleCloseBtn = document.querySelector('#modalScheduleCloseBtn')
var setupBtn = document.querySelector('#setup')
var modalSetup = document.querySelector('#modalSetup')
var modalSetupCloseBtn = document.querySelector('#modalSetupCloseBtn')


scheduleTBtn.onclick = function () {
    modalSchedule.showModal()
}

modalScheduleCloseBtn.onclick = function () {
    modalSchedule.close()
}

setupBtn.onclick = function () {
    modalSetup.showModal()
}

modalSetupCloseBtn.onclick = function () {
    modalSetup.close()
}

