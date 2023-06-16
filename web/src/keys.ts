// import { provide, inject } from 'vue'
import type { InjectionKey, Ref } from 'vue';

interface User {
	username: string
}

export enum PageState {
	add_user,
	login,
	query_profile,
	add_train,
	query_train,
	query_ticket,
	buy_ticket,
	query_order,
	refund_ticket,
	clean,
	exit
};
interface Ticket {
	trainID: string
	from: string,
	to: string,
	leave: string,
	arrive: string,
	price: number,
	seat: number
}

interface Bill {
	trainID: string,
	leave: string,
	from: string,
	to: string
	seat: number
}

export type { User, Ticket, Bill };


export const backend_key = Symbol() as InjectionKey<WebSocket>;

// export const userlist_key = Symbol() as InjectionKey<Set<User>>;

export const current_user_key = Symbol() as InjectionKey<Ref<string>>;

export const pageState_key = Symbol() as InjectionKey<Ref<PageState>>;

export const bill_info_key = Symbol() as InjectionKey<Bill>;
