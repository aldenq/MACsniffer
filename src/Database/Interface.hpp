namespace Database{

    /**
     * Determine if a device exists
     */
    bool deviceExists(MACAdress mac) noexcept ;

    /**
     * Load a device from the mapped cachefile
     * Warning: This operation is only save after a confimed call to 'deviceExists ( ... )'.
     * If there is no device stored with the given mac address it will result in
     * undefined behavior (normally SIGSEGV).
     * @param mac A MACAddress to seach in the file
     * @returns a Promise to the device
     */
    [[nodiscard]]
    async::Promise<Device> getDevice(MACAdress mac);

    /**
     * Write a device to the mapped cachefile.
     * 
     * This call is non-blocking, so deviceExists(...) will
     * not necessarily return true directly after a call to this
     * function, however any other asychronous calls like getDevice
     * are guarenteed to see the results of this write.
     * 
     * The device can be either an existing device or a new device, it
     * does not matter.
     * 
     * @param d A device to write to the mapped cachefile
     * 
     */
    void writeDevice(Device d);

    /**
     * Similar to 'void writeDevice(Device)', writeDeviceSync will write a
     * device to the mapped cachefile (a new or existing device).
     * This is a non-blocking function, but it returns a promise that
     * will be satisfied once the device has been written.
     * 
     * Any subsequent async calls are guarenteed to see the result of this write,
     * but any synchronous calls using 'deviceExists()' are not guarenteed to
     * see the write until after the returned promise has been awaited.
     * 
     * @param d a device to write to the mapped cachefile
     * @returns a promise that is satisfied once d is written
     * 
     */
    [[nodiscard]] 
    async::Promise<bool> writeDeviceSync(Device d);


    [[nodiscard]]
    async::Promise<bool> flushBuffersToFile();

    [[nodiscard]]
    async::Promise<size_t> absoluteCompress();




};