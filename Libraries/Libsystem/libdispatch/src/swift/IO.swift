//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

import CDispatch

extension DispatchIO {

	public enum StreamType : UInt  {
		case stream = 0
		case random = 1
	}

	public struct CloseFlags : OptionSet, RawRepresentable {
		public let rawValue: UInt
		public init(rawValue: UInt) { self.rawValue = rawValue }

		public static let stop = CloseFlags(rawValue: 1)
	}

	public struct IntervalFlags : OptionSet, RawRepresentable {
		public let rawValue: UInt
		public init(rawValue: UInt) { self.rawValue = rawValue }
		public init(nilLiteral: ()) { self.rawValue = 0 }

		public static let strictInterval = IntervalFlags(rawValue: 1)
	}

	public class func read(fromFileDescriptor: Int32, maxLength: Int, runningHandlerOn queue: DispatchQueue, handler: @escaping (_ data: DispatchData, _ error: Int32) -> Void) {
		dispatch_read(dispatch_fd_t(fromFileDescriptor), maxLength, queue.__wrapped) { (data: dispatch_data_t, error: Int32) in
			handler(DispatchData(borrowedData: data), error)
		}
	}

	public class func write(toFileDescriptor: Int32, data: DispatchData, runningHandlerOn queue: DispatchQueue, handler: @escaping (_ data: DispatchData?, _ error: Int32) -> Void) {
		dispatch_write(dispatch_fd_t(toFileDescriptor), data.__wrapped.__wrapped, queue.__wrapped) { (data: dispatch_data_t?, error: Int32) in
			handler(data.map { DispatchData(borrowedData: $0) }, error)
		}
	}

	public convenience init(
		type: StreamType,
		fileDescriptor: Int32,
		queue: DispatchQueue,
		cleanupHandler: @escaping (_ error: Int32) -> Void)
	{
		self.init(__type: type.rawValue, fd: fileDescriptor, queue: queue, handler: cleanupHandler)
	}

	@available(swift, obsoleted: 4)
	public convenience init(
		type: StreamType,
		path: UnsafePointer<Int8>,
		oflag: Int32,
		mode: mode_t,
		queue: DispatchQueue,
		cleanupHandler: @escaping (_ error: Int32) -> Void)
	{
		self.init(__type: type.rawValue, path: path, oflag: oflag, mode: mode, queue: queue, handler: cleanupHandler)
	}

	@available(swift, introduced: 4)
	public convenience init?(
		type: StreamType,
		path: UnsafePointer<Int8>,
		oflag: Int32,
		mode: mode_t,
		queue: DispatchQueue,
		cleanupHandler: @escaping (_ error: Int32) -> Void)
	{
		self.init(__type: type.rawValue, path: path, oflag: oflag, mode: mode, queue: queue, handler: cleanupHandler)
	}

	public convenience init(
		type: StreamType,
		io: DispatchIO,
		queue: DispatchQueue,
		cleanupHandler: @escaping (_ error: Int32) -> Void)
	{
		self.init(__type: type.rawValue, io: io, queue: queue, handler: cleanupHandler)
	}

	public func read(offset: off_t, length: Int, queue: DispatchQueue, ioHandler: @escaping (_ done: Bool, _ data: DispatchData?, _ error: Int32) -> Void) {
		dispatch_io_read(self.__wrapped, offset, length, queue.__wrapped) { (done: Bool, data: dispatch_data_t?, error: Int32) in
			ioHandler(done, data.map { DispatchData(borrowedData: $0) }, error)
		}
	}

	public func write(offset: off_t, data: DispatchData, queue: DispatchQueue, ioHandler: @escaping (_ done: Bool, _ data: DispatchData?, _ error: Int32) -> Void) {
		dispatch_io_write(self.__wrapped, offset, data.__wrapped.__wrapped, queue.__wrapped) { (done: Bool, data: dispatch_data_t?, error: Int32) in
			ioHandler(done, data.map { DispatchData(borrowedData: $0) }, error)
		}
	}

	public func setInterval(interval: DispatchTimeInterval, flags: IntervalFlags = []) {
		dispatch_io_set_interval(self.__wrapped, UInt64(interval.rawValue), dispatch_io_interval_flags_t(flags.rawValue))
	}

	public func close(flags: CloseFlags = []) {
		dispatch_io_close(self.__wrapped, dispatch_io_close_flags_t(flags.rawValue))
	}
}
