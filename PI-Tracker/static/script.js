class Zone {
    constructor(sensor, data) {
        var that = this
        this.sensor = sensor
        this.dmxchannels = data ? data['dmxchannels'] : []
        this.dmxvalue = data ? data['dmxvalue'] : 255
        this.min = data ? data['min'] : 500
        this.max = data ? data['max'] : 1500

        this.elem = $('<div class="zone"></div>')

        var row = $('<div class="row">').appendTo(this.elem)


        // RANGE
        this.i_ranger = $('<div class="ranger" />')
        $('<div class="col s6"></div>').appendTo(row).append(this.i_ranger)
        noUiSlider.create(this.i_ranger.get(0), {
            start: [this.min / 1000.0, this.max / 1000.0],
            connect: true,
            margin: 0.3,
            behaviour: 'tap-drag',
            range: {
                'min': [0.5],
                'max': [10.0]
            },
            tooltips: [wNumb({ decimals: 1 }), wNumb({ decimals: 1 })],
        });
        this.i_ranger.get(0).noUiSlider.on('change', () => {
            that.min = Math.round(that.i_ranger.get(0).noUiSlider.get()[0] * 10) * 100
            that.max = Math.round(that.i_ranger.get(0).noUiSlider.get()[1] * 10) * 100
            saveAll()
        })


        // DMX VALUE
        this.i_value = $('<input type="number" min="0" max="255" value="' + this.dmxvalue + '"/>')
        $('<div class="col s2">@</div>').appendTo(row).append(this.i_value)
        this.i_value.on('change', () => {
            that.dmxvalue = Math.min(that.i_value.val(), 255)
            that.dmxvalue = Math.max(that.dmxvalue, 0)
            if (that.i_value.val() != that.dmxvalue) that.i_value.val(that.dmxvalue)
            saveAll()
        })

        // DMX CHANNELS
        this.i_channels = $('<div class="chips"></div>')
        $('<div class="col s3">CH.</div>').appendTo(row).append(this.i_channels)
        var chips = []
        for (var c of this.dmxchannels) chips.push({ tag: c })
        this.i_channels.chips({
            data: chips,
            onChipAdd: (event, chip) => {
                var value = $(chip).clone().children().remove().end().text()
                var intValue = parseInt(value)
                if ($.isNumeric(value) && intValue > 0 && intValue < 513 && !this.dmxchannels.includes(intValue)) {
                    that.dmxchannels.push(intValue)
                    saveAll()
                } else $(chip).remove();
            },
            onChipDelete: (event, chip) => {
                var value = $(chip).clone().children().remove().end().text()
                var intValue = parseInt(value)
                const index = that.dmxchannels.indexOf(intValue);
                if (index > -1) that.dmxchannels.splice(index, 1);
                saveAll()
            }
        });

        // DELETE
        this.rm_sensor = $('<a class="waves-effect waves-light btn-small red darken-4"><i class="material-icons">close</i></a>')
        $('<div class="col s1"></div>').appendTo(row).append(this.rm_sensor)
        this.rm_sensor.on('click', () => {
            if (!confirm('Supprimer cette zone ?')) return
            that.elem.remove()
            that.sensor.zones[that.id] = null
            saveAll()
        })

        // APPEND TO SCENE
        this.id = this.sensor.zones.length
        this.sensor.zones.push(this)
        this.sensor.elem.append(this.elem)


    }

    export () {
        var exp = {}
        exp['dmxchannels'] = this.dmxchannels
        exp['dmxvalue'] = this.dmxvalue
        exp['min'] = this.min
        exp['max'] = this.max
        return exp
    }

    destroy() {
        this.elem.remove()
    }

}

class Sensor {
    constructor(scene, data) {
        var that = this
        this.scene = scene
        this.hid = data ? data['hid'] : 0
        this.zones = []

        this.elem = $('<div class="sensor"></div>')

        this.i_hid = $('<input class="hid" type="number" min="0" max="99" value="' + this.hid + '" />')
        this.i_hid.on('change', () => {
            that.i_meas.removeClass('meas' + that.hid)
            that.hid = that.i_hid.val()
            that.i_meas.addClass('meas' + that.hid)
            saveAll()
        })

        this.i_meas = $('<div class="measure meas' + this.hid + '"></div>')

        this.add_zone = $('<a class="waves-effect waves-light btn-small green darken-4"><i class="material-icons left">add</i>zone</a>')
        this.add_zone.on('click', () => {
            that.addZone()
            saveAll()
        })

        this.rm_sensor = $('<a class="waves-effect waves-light btn-small red darken-4"><i class="material-icons">close</i></a>')
        this.rm_sensor.on('click', () => {
            if (!confirm('Supprimer ce capteur ?')) return
            that.elem.remove()
            that.scene.sensors[that.id] = null
            saveAll()
        })

        $('<h5>CAPTEUR</h5>')
            .append(this.i_hid)
            .append(this.i_meas)
            .append(this.rm_sensor)
            .append(this.add_zone)
            .appendTo(this.elem)

        // $('<div class="row"> \
        //         <div class="col s6">POSITION</div> \
        //         <div class="col s2">VALEUR</div> \
        //         <div class="col s4">CANAUX DMX</div> \
        //     </div>').appendTo(this.elem)

        if (data)
            for (const z of data['zones']) this.addZone(z)
        else
            this.addZone(0)

        // APPEND TO SCENE
        this.id = this.scene.sensors.length
        this.scene.sensors.push(this)
        this.scene.elem.append(this.elem)

    }

    export () {
        var exp = { 'hid': this.hid, 'zones': [] }
        for (let z of this.zones)
            if (z) exp['zones'].push(z.export())
        return exp
    }

    addZone(data) {
        var zone = new Zone(this, data)
    }

    destroy() {
        for (let z of this.zones)
            if (z) z.destroy()
        this.hid = 0
        this.zones = []
        this.elem.remove()
    }
}

class Scene {
    constructor(id, data) {
        var that = this
        this.id = id
        this.sensors = []

        this.elem = $('<div class="scene" id="scene-' + this.id + '"></div>')

        this.add_sensor = $('<a class="waves-effect waves-light btn-small green darken-4"><i class="material-icons left">add</i>capteur</a>')
        this.add_sensor.on('click', () => {
            that.addSensor()
            saveAll()
        })

        $('<h5>SCENE ' + this.id + '</h5>').append(this.add_sensor).appendTo(this.elem)

        if (data)
            for (const s of data['sensors']) this.addSensor(s)
    }

    export () {
        if (this.sensors.length == 0) return null
        var exp = { 'sensors': [] }
        for (let s of this.sensors)
            if (s) exp['sensors'].push(s.export())
        return exp
    }

    addSensor(data) {
        var sensor = new Sensor(this, data)
    }

    destroy() {
        for (let s of this.sensors)
            if (s) s.destroy()
        this.sensors = []
        this.elem.remove()
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
        $("input[type='number']").click(function() {
            $(this).select();
        });
    }

    save() {
        var exp = { 'scenes': [] }
        for (let s of this.scenes)
            if (s) exp['scenes'].push(s.export())
            else exp['scenes'].push(null)
        socket.emit('save', exp)
        console.log('save', exp)
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


// SAVE LOGIC
//
var saveTO = null

function saveAll() {
    if ($('.constate').hasClass('connected'))
        $('.constate').addClass('saving')
    clearTimeout(saveTO)
    saveTO = setTimeout(() => {
        Book.save()
        console.log('saved')
        $('.constate').removeClass('saving')
    }, 2000)
}

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
    $('.constate').removeClass('saving')
});

socket.on('scenario', function(data) {
    console.log('scenario', data)
    Book.import(data)
});

socket.on('update', function(data) {

    console.log('update', data)

    if ('scene' in data) {
        $('.scenesel').removeClass('active')
        $('.scenesel[data-value="' + data['scene'] + '"]').addClass('active')
        if (Book.activeScene < 0) $('.scenesel[data-value="' + data['scene'] + '"]').click()
        Book.activeScene = data['scene']
    }

    if ('CAN' in data) {
        if (data['CAN']) $('#CANstatus').addClass('active')
        else $('#CANstatus').removeClass('active')
    }

    if ('DMX' in data) {
        if (data['DMX']) $('#DMXstatus').addClass('active')
        else $('#DMXstatus').removeClass('active')
    }

    if ('CTRL' in data) {
        $("#scenectrl").text(' (dmx ' + data['CTRL'] + ')')
    }

    if ('MEAS' in data) {
        var txt = ''
        $('.measure').text('')
        for (var k in data['MEAS']) {
            txt = data['MEAS'][k]['sensor'] + ':' + data['MEAS'][k]['value'] + ' '
            $('.meas' + data['MEAS'][k]['sensor']).text(Math.round(data['MEAS'][k]['value'] / 10) / 100)
        }
        $("#MEASstatus").text(txt)
        $("#MEASstatus").addClass('active')
    }

});