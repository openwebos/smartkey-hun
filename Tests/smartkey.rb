#!/usr/bin/ruby

# @@@LICENSE
#
#      Copyright (c) 2010 - 2012 Hewlett-Packard Development Company, L.P.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@


# sudo apt-get install libjson-ruby libjson-ruby-doc
require 'json'

module SmartKey

	class Guess
		attr_accessor :str, :sp, :autoAccept, :autoReplace
		def initialize(str)
			self.str = str
			self.sp = false
			self.autoAccept = false
			self.autoReplace = false
		end

		def to_s
			str = "{ \"str\": \"#{self.str}\", \"sp\": #{self.sp}"
			if self.autoAccept then
				str += ", \"auto-accept\": #{self.autoAccept}"
			end
			if self.autoReplace then
				str += ", \"auto-replace\": #{self.autoReplace}"
			end
			str += " }"
			return str
		end
	end

	class SpellCheckResult
		attr_accessor :spelledCorrectly, :guesses

		def initialize(spelledCorrectly)
			self.spelledCorrectly = spelledCorrectly
			self.guesses = Array.new
		end

		def to_s
			str = "{ \"spelledCorrectly\": #{self.spelledCorrectly}, \"guesses\": [ "
			first = true
			self.guesses.each {|guess|
				str += ", " if not first
				first = false
				str += guess.to_s
			}
			str += "] }"
			return str
		end

		def guesses_s
			str = ""
			first = true
			guesses.each {|guess|
				if first then
					first = false
				else
					str += ", "
				end
				str += guess.str
			}
			return str
		end
	end

	class SmartKeyProxy
		def checkSpelling(word)

			query = Hash.new
			query["query"] = word
			
			q = JSON.generate query
			
			# Encode the single quote because we will single quote the string
			encoded = q.gsub(/'/, '\u0027')

			cmd = "luna-send -n 1 palm://com.palm.smartKey/search '#{encoded}'"

			result = nil

			IO.popen(cmd) { |f|
				response = JSON.parse f.gets
				if response["returnValue"] then
					result = SpellCheckResult.new(response["spelledCorrectly"])
					guesses = response["guesses"]
					if guesses then
						guesses.each {|resultGuess|
							guess = Guess.new(resultGuess["str"])

							guess.sp = resultGuess["sp"]
							guess.autoAccept = resultGuess["auto-accept"]
							guess.autoReplace = resultGuess["auto-replace"]

							result.guesses.push guess
						}
					end
				else
					raise "call failure"
				end
			}

			return result
		end

		def setLocale(locale)
			params = Hash.new
			params["locale"] = locale
			
			p = JSON.generate params
			
			# Encode the single quote because we will single quote the string
			encoded = p.gsub(/'/, '\u0027')

			cmd = "luna-send -n 1 palm://com.palm.smartKey/setLocale '#{encoded}'"

			IO.popen(cmd) { |f|
				response = JSON.parse f.gets
				if response["returnValue"] then
				else
					raise "call failure"
				end
			}
		end
	end
end
