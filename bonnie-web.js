class SmartBonnie {
    constructor() {
        this.soundCharacteristic = null;
        this.volume = 20;
    }

    async connect() {
        console.log('Requesting Bluetooth Device...');
        try {
            const device = await navigator.bluetooth.requestDevice({
                filters: [{ services: [0xff09] }], optionalServices: ['battery_service']
            });
            console.log('> Found ' + device.name);
            console.log('Connecting to GATT Server...');
            await device.gatt.connect();
            console.log('Getting Service 0xff09 - Bonnie control...');
            const service = await device.gatt.getPrimaryService(0xff09);
            console.log('Getting Characteristic 0xff0a - Sound control...');
            this.soundCharacteristic = await service.getCharacteristic(0xff0a);
            console.log('All ready!');
        } catch (err) {
            console.error('Connection failed', err);
        }
    }

    playSound(soundId) {
        console.log('Playing sound', soundId);
        return this.soundCharacteristic.writeValue(new Int8Array([soundId & 0xff, (soundId << 8) & 0xff, this.volume]))
            .catch(err => console.log('Error when writing value! ', err));
    }

    setVolume(value) {
        this.volume = value;
    }
}

window.bonnie = new SmartBonnie();
