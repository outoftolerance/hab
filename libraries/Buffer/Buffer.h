#ifndef Buffer_h
#define Buffer_h

/**
 * @brief      Class for Buffer!
 */
class Buffer
{
public:
	/**
	 * @brief      Constructs the buffer
	 *
	 * @param[in]  size  The size of the buffer
	 */
	Buffer(const int size);

	/**
	 * @brief      Destroys the buffer
	 */
	~Buffer();

	/**
	 * @brief      Push a new char onto the buffer
	 *
	 * @param[in]  c     The new char to be pushed onto the buffer
	 *
	 * @return     True on success, false on fail
	 */
	bool push(char c);

	/**
	 * @brief      Pops a char off the buffer
	 *
	 * @return     The char popped from the buffer, NULL returned if not data available
	 */
	char pop();

	/**
	 * @brief      Returns the next char off the buffer without popping it
	 *
	 * @return     The next char on the buffer
	 */
	char peek();

	/**
	 * @brief      Function to check if there is data in the buffer
	 *
	 * @return     True if there is data available in the buffer, false if not
	 */
	bool available();
private:
	int front_;		/**< Marks the front of the buffer */
	int back_;		/**< Marks the back of the buffer */
	int size_;		/**< Marks the size of the buffer */
	char* store_;	/**< Char pointer to the char data store array */
};

#endif
