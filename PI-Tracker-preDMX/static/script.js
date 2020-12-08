class Zone {
    constructor(id, data) {
        this.id = id
        this.dmxchannels = data['dmxchannels']
        this.dmxvalue = data['dmxvalue']
        this.min = data['min']
        this.max = data['max']

        this.elem = $('<div class="zone"></div>')

        var row = $('<div class="row">').appendTo(this.elem)

        this.i_channels = $('<div class="chips"></div>')
        $('<div class="col s4"></div>').appendTo(row).append(this.i_channels)

        this.i_value = $('<input type="number" min="0" max="255" value="' + this.dmxvalue + '"/>')
        $('<div class="col s2">@</div>').appendTo(row).append(this.i_value)

        this.i_ranger = $('<input type="range" min="0" max="1000"  />')
        $('<div class="col s6"></div>').appendTo(row).append(this.i_ranger)
    }

    export () {
        exp = {}
        exp['dmxchannels'] = this.dmxchannels
        exp['dmxvalue'] = this.dmxvalue
        exp['min'] = this.min
        exp['max'] = this.max
        return exp
    }

    destroy() {

    }

    element() {

    }
}

class Sensor {
    constructor(id, data) {
        this.id = id
        this.hid = data['hid']
        this.zones = []

        this.elem = $('<div class="sensor"></div>')

        this.i_hid = $('<input class="hid" type="number" min="0" max="99" value="' + this.hid + '" />')

        $('<h5>CAPTEUR</h5>').append(this.i_hid).appendTo(this.elem)

        $('<div class="row"> \
                <div class="col s4">CANAUX DMX</div> \
                <div class="col s2">VALEUR</div> \
                <div class="col s6">POSITION</div> \
            </div>').appendTo(this.elem)

        for (const [i, z] of data['zones'].entries()) this.addZone(i, z)
    }

    export () {
        exp = { 'hid': this.hid, 'zones': [] }
        for (let z of this.zones)
            exp['zones'].push(z.export())
        return exp
    }

    addZone(i, data) {
        var zone = new Zone(i, data)
        this.zones.push(zone)

        this.elem.append(zone.elem)
    }

    destroy() {
        for (let z of this.zones) z.destroy()
        this.hid = 0
        this.zones = []
    }
}

class Scene {
    constructor(id, data) {
        this.id = id
        this.sensors = []

        this.elem = $('<div class="scene" id="scene-' + this.id + '"><h5>SCENE ' + this.id + '</h5></div>')

        if (data)
            for (const [i, s] of data['sensors'].entries()) this.addSensor(i, s)
    }

    export () {
        exp = { 'sensors': [] }
        for (let s of this.sensors)
            exp['sensors'].push(s.export())
        return exp
    }

    addSensor(i, data) {
        var sensor = new Sensor(i, data)
        this.sensors.push(sensor)

        this.elem.append(sensor.elem)
    }

    destroy() {
        for (let s of this.sensors) s.destroy()
        this.sensors = []
    }
}


class SceneBook {
    constructor() {
        this.activeScene = -1
        this.editScene = 1

        this.scenes = []

        this.elem = $('<div class="book"></div>')
    }

    import (data) {
        for (let s of this.scenes)
            if (s) s.destroy()
        this.scenes = []
        this.elem.empty()

        for (const [i, s] of data['scenes'].entries()) {
            var scene = new Scene(i, s)
            this.scenes.push(scene)
            this.elem.append(scene.elem)
        }

        // UI init
        this.edit(this.editScene)
        $('.chips').chips();
        $("input[type='number']").click(function() {
            $(this).select();
        });
    }

    save() {
        exp = { 'scenes': [] }
        for (let s of this.scenes)
            if (s) exp['scenes'].push(s.export())
            else exp['scenes'].push(null)
        socket.emit('save', exp)
        console.log('save', data)
    }

    edit(sI) {
        $('.scene').hide();
        if (sI > 0 && sI < this.scenes.length)
            $('#scene-' + sI).show()
        this.editScene = sI
    }

}

var Book = new SceneBook()
$('#content').append(Book.elem)


// SOCKETIO
// 
var wsURL = 'http://localhost:5000'

console.log('connecting to ', wsURL)
var socket = io(wsURL);

socket.on('connect', function() {
    $('.constate').addClass('connected')
    console.log('connected!')
});

socket.on('disconnect', function() {
    console.log('disconnected:(')
    $('.constate').removeClass('connected')
});

socket.on('scenario', function(data) {
    console.log('scenario', data)
    Book.import(data)
});





// MENU
// 
$(document).ready(function() {

    $('.sidenav').sidenav()
    $('.collapsible').collapsible()

    // fill Scenes Table
    for (var i = 0; i < 20; i++) {
        var tr = $('<tr>')
        for (var j = 1; j <= 5; j++)
            $('<td class="scenesel">').html(j + i * 5).attr("data-value", j + i * 5).appendTo(tr)
        $('#scene-table').append(tr)
    }

    // bind Scenes select
    $('.scenesel').on('click', (e) => {
        var el = $(e.currentTarget)
        $('.scenesel').removeClass('edit')
        el.addClass('edit')

        Book.edit(el.attr("data-value"))
    })


});